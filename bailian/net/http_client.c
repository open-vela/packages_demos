#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <nuttx/config.h>

#include "c_utils/c_utils.h"

#ifdef CONFIG_CRYPTO_MBEDTLS
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#endif
#include "cJSON.h"

#define HTTP_DEFAULT_PORT 80
#define HTTPS_DEFAULT_PORT 443

struct http_url_s
{
  char scheme[6];
  char host[128];
  char path[256];
  uint16_t port;
  bool tls;
};

struct http_tls_s
{
#ifdef CONFIG_CRYPTO_MBEDTLS
  mbedtls_net_context net;
  mbedtls_ssl_context ssl;
  mbedtls_ssl_config conf;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_entropy_context entropy;
#endif
  bool inited;
};

static int parse_url(const char *url, struct http_url_s *out)
{
  const char *host_start;
  const char *path_start;
  const char *port_start;
  size_t host_len;
  size_t path_len;

  if (url == NULL || out == NULL)
    {
      return -EINVAL;
    }

  memset(out, 0, sizeof(*out));

  if (strncmp(url, "http://", 7) == 0)
    {
      strcpy(out->scheme, "http");
      out->port = HTTP_DEFAULT_PORT;
      out->tls = false;
      host_start = url + 7;
    }
  else if (strncmp(url, "https://", 8) == 0)
    {
      strcpy(out->scheme, "https");
      out->port = HTTPS_DEFAULT_PORT;
      out->tls = true;
      host_start = url + 8;
    }
  else
    {
      return -EINVAL;
    }

  path_start = strchr(host_start, '/');
  if (path_start == NULL)
    {
      path_start = host_start + strlen(host_start);
    }

  port_start = memchr(host_start, ':', (size_t)(path_start - host_start));
  if (port_start != NULL)
    {
      host_len = (size_t)(port_start - host_start);
      out->port = (uint16_t)strtoul(port_start + 1, NULL, 10);
    }
  else
    {
      host_len = (size_t)(path_start - host_start);
    }

  if (host_len == 0 || host_len >= sizeof(out->host))
    {
      return -EINVAL;
    }

  memcpy(out->host, host_start, host_len);
  out->host[host_len] = '\0';

  if (*path_start == '\0')
    {
      strcpy(out->path, "/");
    }
  else
    {
      path_len = strlen(path_start);
      if (path_len >= sizeof(out->path))
        {
          return -EINVAL;
        }
      memcpy(out->path, path_start, path_len + 1);
    }

  return 0;
}

static int tcp_connect(const char *host, uint16_t port, int timeout_ms)
{
  struct addrinfo hints;
  struct addrinfo *res = NULL;
  struct addrinfo *it;
  char port_str[8];
  int fd = -1;
  int ret;

  snprintf(port_str, sizeof(port_str), "%u", port);
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  ret = getaddrinfo(host, port_str, &hints, &res);
  if (ret != 0)
    {
      return -EHOSTUNREACH;
    }

  for (it = res; it != NULL; it = it->ai_next)
    {
      struct timeval tv;

      fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
      if (fd < 0)
        {
          continue;
        }

      tv.tv_sec = timeout_ms / 1000;
      tv.tv_usec = (timeout_ms % 1000) * 1000;
      setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
      setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

      ret = connect(fd, it->ai_addr, it->ai_addrlen);
      if (ret == 0)
        {
          break;
        }

      close(fd);
      fd = -1;
    }

  freeaddrinfo(res);
  return fd;
}

static ssize_t socket_write(int fd, const uint8_t *data, size_t len)
{
  size_t sent = 0;

  while (sent < len)
    {
      ssize_t ret = send(fd, data + sent, len - sent, 0);
      if (ret < 0)
        {
          if (errno == EINTR)
            {
              continue;
            }
          return -errno;
        }
      sent += (size_t)ret;
    }

  return (ssize_t)sent;
}

static ssize_t socket_read(int fd, uint8_t *data, size_t len)
{
  ssize_t ret = recv(fd, data, len, 0);
  if (ret < 0 && errno == EINTR)
    {
      return 0;
    }
  return ret;
}

#ifdef CONFIG_CRYPTO_MBEDTLS

/* Custom recv with poll() timeout for NuttX FDCHECK compatibility */

static int http_net_recv(void *ctx, unsigned char *buf, size_t len)
{
  mbedtls_net_context *net_ctx = (mbedtls_net_context *)ctx;
  struct pollfd pfd;
  int ret;

  pfd.fd = net_ctx->fd;
  pfd.events = POLLIN;
  pfd.revents = 0;

  ret = poll(&pfd, 1, 10000);

  if (ret == 0)
    {
      return MBEDTLS_ERR_SSL_TIMEOUT;
    }

  if (ret < 0)
    {
      return MBEDTLS_ERR_NET_RECV_FAILED;
    }

  if (pfd.revents & (POLLERR | POLLHUP))
    {
      return MBEDTLS_ERR_NET_CONN_RESET;
    }

  ret = (int)read(net_ctx->fd, buf, len);

  if (ret < 0)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        {
          return MBEDTLS_ERR_SSL_WANT_READ;
        }

      if (errno == EPIPE || errno == ECONNRESET)
        {
          return MBEDTLS_ERR_NET_CONN_RESET;
        }

      return MBEDTLS_ERR_NET_RECV_FAILED;
    }

  return ret;
}

/* IPv4-only TCP connect to avoid FDCHECK + select() issues */

static int http_tcp_connect_ipv4(const char *host, uint16_t port,
                                 int timeout_ms)
{
  struct addrinfo hints;
  struct addrinfo *res = NULL;
  struct addrinfo *it;
  char port_str[8];
  struct timeval tv;
  int fd = -1;
  int ret;

  snprintf(port_str, sizeof(port_str), "%u", port);
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  ret = getaddrinfo(host, port_str, &hints, &res);
  if (ret != 0)
    {
      return -EHOSTUNREACH;
    }

  for (it = res; it != NULL; it = it->ai_next)
    {
      fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
      if (fd < 0)
        {
          continue;
        }

      tv.tv_sec = timeout_ms / 1000;
      tv.tv_usec = (timeout_ms % 1000) * 1000;
      setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
      setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

      ret = connect(fd, it->ai_addr, it->ai_addrlen);
      if (ret == 0)
        {
          break;
        }

      close(fd);
      fd = -1;
    }

  freeaddrinfo(res);
  return fd;
}

static int tls_connect(struct http_tls_s *tls, const char *host,
                       uint16_t port, int timeout_ms)
{
  int ret;
  int fd;

  if (tls == NULL)
    {
      return -EINVAL;
    }

  mbedtls_net_init(&tls->net);
  mbedtls_ssl_init(&tls->ssl);
  mbedtls_ssl_config_init(&tls->conf);
  mbedtls_ctr_drbg_init(&tls->ctr_drbg);
  mbedtls_entropy_init(&tls->entropy);

  ret = mbedtls_ctr_drbg_seed(&tls->ctr_drbg, mbedtls_entropy_func,
                              &tls->entropy, (const unsigned char *)"bailian",
                              strlen("bailian"));
  if (ret != 0)
    {
      return -EIO;
    }

  /* Use custom IPv4-only connect instead of mbedtls_net_connect
   * to avoid internal select()/FD_SET issues with FDCHECK.
   */

  fd = http_tcp_connect_ipv4(host, port, timeout_ms);
  if (fd < 0)
    {
      return -EHOSTUNREACH;
    }

  tls->net.fd = fd;

  ret = mbedtls_ssl_config_defaults(&tls->conf, MBEDTLS_SSL_IS_CLIENT,
                                    MBEDTLS_SSL_TRANSPORT_STREAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT);
  if (ret != 0)
    {
      return -EIO;
    }

  mbedtls_ssl_conf_authmode(&tls->conf, MBEDTLS_SSL_VERIFY_NONE);
  mbedtls_ssl_conf_rng(&tls->conf, mbedtls_ctr_drbg_random, &tls->ctr_drbg);
  if (timeout_ms > 0)
    {
      mbedtls_ssl_conf_read_timeout(&tls->conf, timeout_ms);
    }

  ret = mbedtls_ssl_setup(&tls->ssl, &tls->conf);
  if (ret != 0)
    {
      return -EIO;
    }

  ret = mbedtls_ssl_set_hostname(&tls->ssl, host);
  if (ret != 0)
    {
      return -EIO;
    }

  /* Use custom recv with poll() to avoid select()/FD_SET overflow
   * caused by FDCHECK-encoded fd values exceeding FD_SETSIZE.
   */

  mbedtls_ssl_set_bio(&tls->ssl, &tls->net, mbedtls_net_send,
                      http_net_recv, NULL);

  do
    {
      ret = mbedtls_ssl_handshake(&tls->ssl);
    }
  while (ret == MBEDTLS_ERR_SSL_WANT_READ ||
         ret == MBEDTLS_ERR_SSL_WANT_WRITE);

  if (ret != 0)
    {
      return -EIO;
    }

  tls->inited = true;
  return 0;
}

static void tls_close(struct http_tls_s *tls)
{
  if (tls == NULL || !tls->inited)
    {
      return;
    }

  mbedtls_ssl_close_notify(&tls->ssl);
  mbedtls_net_free(&tls->net);
  mbedtls_ssl_free(&tls->ssl);
  mbedtls_ssl_config_free(&tls->conf);
  mbedtls_ctr_drbg_free(&tls->ctr_drbg);
  mbedtls_entropy_free(&tls->entropy);
  tls->inited = false;
}

static ssize_t tls_write(struct http_tls_s *tls, const uint8_t *data,
                         size_t len)
{
  size_t sent = 0;

  while (sent < len)
    {
      int ret = mbedtls_ssl_write(&tls->ssl, data + sent, len - sent);
      if (ret < 0)
        {
          if (ret == MBEDTLS_ERR_SSL_WANT_READ ||
              ret == MBEDTLS_ERR_SSL_WANT_WRITE)
            {
              continue;
            }
          return -EIO;
        }
      sent += (size_t)ret;
    }

  return (ssize_t)sent;
}

static ssize_t tls_read(struct http_tls_s *tls, uint8_t *data, size_t len)
{
  int ret = mbedtls_ssl_read(&tls->ssl, data, len);
  if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
    {
      return 0;
    }
  if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
    {
      return 0;
    }
  if (ret < 0)
    {
      return -EIO;
    }
  return ret;
}
#endif

static const char *find_header_case(const char *haystack, const char *needle)
{
  size_t needle_len;
  const char *p;

  if (haystack == NULL || needle == NULL)
    {
      return NULL;
    }

  needle_len = strlen(needle);
  for (p = haystack; *p != '\0'; p++)
    {
      if (strncasecmp(p, needle, needle_len) == 0)
        {
          return p;
        }
    }

  return NULL;
}

static int http_read_response(int fd, struct http_tls_s *tls, bool use_tls,
                              char *response, size_t response_len)
{
  uint8_t buffer[512];
  char *header_end = NULL;
  size_t header_len = 0;
  size_t body_len = 0;
  size_t content_len = 0;
  bool has_content_len = false;

  while (true)
    {
      ssize_t ret = use_tls
#ifdef CONFIG_CRYPTO_MBEDTLS
                        ? tls_read(tls, buffer, sizeof(buffer))
#else
                        ? -ENOTSUP
#endif
                        : socket_read(fd, buffer, sizeof(buffer));
      if (ret < 0)
        {
          return (int)ret;
        }
      if (ret == 0)
        {
          break;
        }

      if (header_end == NULL)
        {
          size_t copy_len = (size_t)ret;
          if (header_len + copy_len >= response_len)
            {
              return -ENOSPC;
            }

          memcpy(response + header_len, buffer, copy_len);
          header_len += copy_len;
          response[header_len] = '\0';
          header_end = strstr(response, "\r\n\r\n");
          if (header_end != NULL)
            {
              const char *len_str = find_header_case(response, "Content-Length:");
              if (len_str != NULL)
                {
                  len_str += strlen("Content-Length:");
                  while (*len_str == ' ') len_str++;
                  content_len = (size_t)strtoul(len_str, NULL, 10);
                  has_content_len = true;
                }

              body_len = header_len - (size_t)(header_end - response) - 4;
              memmove(response, header_end + 4, body_len);
              header_len = 0;
              response[body_len] = '\0';

              if (has_content_len && body_len >= content_len)
                {
                  return (int)body_len;
                }
            }
        }
      else
        {
          size_t copy_len = (size_t)ret;
          if (body_len + copy_len >= response_len)
            {
              return -ENOSPC;
            }
          memcpy(response + body_len, buffer, copy_len);
          body_len += copy_len;
          response[body_len] = '\0';
          if (has_content_len && body_len >= content_len)
            {
              return (int)body_len;
            }
        }
    }

  return (int)body_len;
}

int bailian_http_post(const char *url, const char *content_type,
                      const char *payload, char *response,
                      size_t response_len, int timeout_ms)
{
  struct http_url_s parsed;
  char request[512];
  int fd = -1;
  int ret;
  size_t payload_len = payload != NULL ? strlen(payload) : 0;
  ssize_t sent;
  struct http_tls_s tls;

  if (parse_url(url, &parsed) != 0)
    {
      return -EINVAL;
    }

  memset(&tls, 0, sizeof(tls));

  if (parsed.tls)
    {
#ifdef CONFIG_CRYPTO_MBEDTLS
      ret = tls_connect(&tls, parsed.host, parsed.port, timeout_ms);
      if (ret < 0)
        {
          return ret;
        }
#else
      return -ENOTSUP;
#endif
    }
  else
    {
      fd = tcp_connect(parsed.host, parsed.port, timeout_ms);
      if (fd < 0)
        {
          return fd;
        }
    }

  snprintf(request, sizeof(request),
           "POST %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Connection: close\r\n"
           "Content-Type: %s\r\n"
           "Content-Length: %u\r\n"
           "\r\n",
           parsed.path, parsed.host,
           content_type != NULL ? content_type : "application/json",
           (unsigned int)payload_len);

  if (parsed.tls)
    {
#ifdef CONFIG_CRYPTO_MBEDTLS
      sent = tls_write(&tls, (const uint8_t *)request, strlen(request));
#else
      sent = -ENOTSUP;
#endif
    }
  else
    {
      sent = socket_write(fd, (const uint8_t *)request, strlen(request));
    }

  if (sent < 0)
    {
      ret = (int)sent;
      goto out;
    }

  if (payload_len > 0)
    {
      if (parsed.tls)
        {
#ifdef CONFIG_CRYPTO_MBEDTLS
          sent = tls_write(&tls, (const uint8_t *)payload, payload_len);
#else
          sent = -ENOTSUP;
#endif
        }
      else
        {
          sent = socket_write(fd, (const uint8_t *)payload, payload_len);
        }

      if (sent < 0)
        {
          ret = (int)sent;
          goto out;
        }
    }

  ret = http_read_response(fd, &tls, parsed.tls, response, response_len);

out:
  if (parsed.tls)
    {
#ifdef CONFIG_CRYPTO_MBEDTLS
      tls_close(&tls);
#endif
    }
  else if (fd >= 0)
    {
      close(fd);
    }

  return ret;
}

/* Extract 'data' field from JSON response for SDK parsing */
static int extract_json_data(const char *json_str, char *data_out, size_t data_len)
{
  cJSON *root = NULL;
  cJSON *data = NULL;
  char *data_str = NULL;
  int ret = -1;

  root = cJSON_Parse(json_str);
  if (root == NULL)
    {
      return -1;
    }

  data = cJSON_GetObjectItem(root, "data");
  if (data == NULL)
    {
      cJSON_Delete(root);
      return -1;
    }

  data_str = cJSON_PrintUnformatted(data);
  if (data_str != NULL)
    {
      size_t len = strlen(data_str);
      if (len < data_len)
        {
          memcpy(data_out, data_str, len + 1);
          ret = (int)len;
        }
      cJSON_free(data_str);
    }

  cJSON_Delete(root);
  return ret;
}

int bailian_http_register(const char *url, const char *req,
                          char *rsp, size_t rsp_len)
{
  char raw_rsp[2048];
  int ret;

  ret = bailian_http_post(url, "application/json", req, raw_rsp,
                          sizeof(raw_rsp), 10000);
  if (ret <= 0)
    {
      return ret;
    }

  /* Extract 'data' field from JSON response per official doc */
  ret = extract_json_data(raw_rsp, rsp, rsp_len);
  return ret;
}

int bailian_http_login(const char *url, const char *req,
                       char *rsp, size_t rsp_len)
{
  char raw_rsp[2048];
  int ret;

  ret = bailian_http_post(url, "application/json", req, raw_rsp,
                          sizeof(raw_rsp), 10000);
  if (ret <= 0)
    {
      return ret;
    }

  /* Extract 'data' field from JSON response per official doc */
  ret = extract_json_data(raw_rsp, rsp, rsp_len);
  return ret;
}
