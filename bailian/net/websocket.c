#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include <nuttx/config.h>

#ifdef CONFIG_CRYPTO_MBEDTLS
#include <mbedtls/ctr_drbg.h>
#ifdef CONFIG_MBEDTLS_DEBUG_C
#include <mbedtls/debug.h>
#endif
#include <mbedtls/entropy.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <mbedtls/x509_crt.h>

#ifdef CONFIG_MBEDTLS_DEBUG_C
static void my_debug(void *ctx, int level, const char *file, int line,
                     const char *str)
{
  (void)ctx;
  (void)level;
}
#endif

/* GlobalSign Root CA - R3 (PEM format)
 * Valid: 2009-03-18 to 2029-03-18
 * Required by Alibaba Bailian/DashScope TLS connection
 */

static const char globalsign_root_r3_pem[] =
  "-----BEGIN CERTIFICATE-----\r\n"
  "MIIDXzCCAkegAwIBAgILBAAAAAABIVhTCKIwDQYJKoZIhvcNAQELBQAwTDEgMB4G\r\n"
  "A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjMxEzARBgNVBAoTCkdsb2JhbFNp\r\n"
  "Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDkwMzE4MTAwMDAwWhcNMjkwMzE4\r\n"
  "MTAwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMzETMBEG\r\n"
  "A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\r\n"
  "hvcNAQEBBQADggEPADCCAQoCggEBAMwldpB5BngiFvXAg7aEyiie/QV2EcWtiHL8\r\n"
  "RgJDx7KKnQRfJMsuS+FggkbhUqsMgUdwbN1k0ev1LKMPgj0MK66X17YUhhB5uzsT\r\n"
  "gHeMCOFJ0mpiLx9e+pZo34knlTifBtc+ycsmWQ1z3rDI6SYOgxXG71uL0gRgykmm\r\n"
  "KPZpO/bLyCiR5Z2KYVc3rHQU3HTgOu5yLy6c+9C7v/U9AOEGM+iCK65TpjoWc4zd\r\n"
  "QQ4gOsC0p6Hpsk+QLjJg6VfLuQSSaGjlOCZgdbKfd/+RFO+uIEn8rUAVSNECMWEZ\r\n"
  "XriX7613t2Saer9fwRPvm2L7DWzgVGkWqQPabumDk3F2xmmFghcCAwEAAaNCMEAw\r\n"
  "DgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFI/wS3+o\r\n"
  "LkUkrk1Q+mOai97i3Ru8MA0GCSqGSIb3DQEBCwUAA4IBAQBLQNvAUKr+yAzv95ZU\r\n"
  "RUm7lgAJQayzE4aGKAczymvmdLm6AC2upArT9fHxD4q/c2dKg8dEe3jgr25sbwMp\r\n"
  "jjM5RcOO5LlXbKr8EpbsU8Yt5CRsuZRj+9xTaGdWPoO4zzUhw8lo/s7awlOqzJCK\r\n"
  "6fBdRoyV3XpYKBovHd7NADdBj+1EbddTKJd+82cEHhXXipa0095MJ6RMG3NzdvQX\r\n"
  "mcIfeg7jLQitChws/zyrVQ4PkX4268NXSb7hLi18YIvDQVETI53O9zJrlAGomecs\r\n"
  "Mx86OyXShkDOOyyGeMlhLxS67ttVb9+E7gUJTb0o2HLO02JQZR7rkpeDMdmztcpH\r\n"
  "WD9f\r\n"
  "-----END CERTIFICATE-----\r\n";
#endif

#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

struct ws_url_s
{
  char host[128];
  char path[256];
  uint16_t port;
  bool tls;
};

struct ws_tls_s
{
#ifdef CONFIG_CRYPTO_MBEDTLS
  mbedtls_net_context net;
  mbedtls_ssl_context ssl;
  mbedtls_ssl_config conf;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_entropy_context entropy;
  mbedtls_x509_crt cacert;
#endif
  bool inited;
};

struct bailian_ws_client
{
  int fd;
  bool tls;
  struct ws_tls_s tls_ctx;
};

typedef struct
{
  uint32_t state[5];
  uint64_t count;
  uint8_t buffer[64];
} sha1_ctx_t;

static void sha1_transform(uint32_t state[5], const uint8_t buffer[64])
{
  uint32_t a, b, c, d, e, t;
  uint32_t w[80];
  int i;

  for (i = 0; i < 16; i++)
    {
      w[i] = ((uint32_t)buffer[i * 4] << 24) |
             ((uint32_t)buffer[i * 4 + 1] << 16) |
             ((uint32_t)buffer[i * 4 + 2] << 8) |
             (uint32_t)buffer[i * 4 + 3];
    }

  for (i = 16; i < 80; i++)
    {
      uint32_t v = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
      w[i] = (v << 1) | (v >> 31);
    }

  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];
  e = state[4];

  for (i = 0; i < 80; i++)
    {
      uint32_t f;
      uint32_t k;

      if (i < 20)
        {
          f = (b & c) | ((~b) & d);
          k = 0x5A827999;
        }
      else if (i < 40)
        {
          f = b ^ c ^ d;
          k = 0x6ED9EBA1;
        }
      else if (i < 60)
        {
          f = (b & c) | (b & d) | (c & d);
          k = 0x8F1BBCDC;
        }
      else
        {
          f = b ^ c ^ d;
          k = 0xCA62C1D6;
        }

      t = ((a << 5) | (a >> 27)) + f + e + k + w[i];
      e = d;
      d = c;
      c = (b << 30) | (b >> 2);
      b = a;
      a = t;
    }

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
}

static void sha1_init(sha1_ctx_t *ctx)
{
  ctx->state[0] = 0x67452301;
  ctx->state[1] = 0xEFCDAB89;
  ctx->state[2] = 0x98BADCFE;
  ctx->state[3] = 0x10325476;
  ctx->state[4] = 0xC3D2E1F0;
  ctx->count = 0;
}

static void sha1_update(sha1_ctx_t *ctx, const uint8_t *data, size_t len)
{
  size_t i;
  size_t index = (size_t)((ctx->count >> 3) & 0x3f);

  ctx->count += (uint64_t)len << 3;

  for (i = 0; i < len; i++)
    {
      ctx->buffer[index++] = data[i];
      if (index == 64)
        {
          sha1_transform(ctx->state, ctx->buffer);
          index = 0;
        }
    }
}

static void sha1_final(sha1_ctx_t *ctx, uint8_t digest[20])
{
  uint8_t pad[64];
  uint8_t len_bytes[8];
  size_t index;
  size_t pad_len;
  int i;

  memset(pad, 0, sizeof(pad));
  pad[0] = 0x80;

  index = (size_t)((ctx->count >> 3) & 0x3f);
  pad_len = (index < 56) ? (56 - index) : (120 - index);

  for (i = 0; i < 8; i++)
    {
      len_bytes[7 - i] = (uint8_t)(ctx->count >> (i * 8));
    }

  sha1_update(ctx, pad, pad_len);
  sha1_update(ctx, len_bytes, 8);

  for (i = 0; i < 5; i++)
    {
      digest[i * 4] = (uint8_t)(ctx->state[i] >> 24);
      digest[i * 4 + 1] = (uint8_t)(ctx->state[i] >> 16);
      digest[i * 4 + 2] = (uint8_t)(ctx->state[i] >> 8);
      digest[i * 4 + 3] = (uint8_t)(ctx->state[i]);
    }
}

static void base64_encode(const uint8_t *src, size_t len, char *dst,
                          size_t dst_len)
{
  static const char table[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  size_t i;
  size_t o = 0;

  for (i = 0; i < len && o + 4 < dst_len; i += 3)
    {
      uint32_t a = src[i];
      uint32_t b = (i + 1 < len) ? src[i + 1] : 0;
      uint32_t c = (i + 2 < len) ? src[i + 2] : 0;

      dst[o++] = table[(a >> 2) & 0x3f];
      dst[o++] = table[((a & 0x3) << 4) | ((b >> 4) & 0x0f)];
      if (i + 1 < len)
        {
          dst[o++] = table[((b & 0x0f) << 2) | ((c >> 6) & 0x03)];
        }
      else
        {
          dst[o++] = '=';
        }
      if (i + 2 < len)
        {
          dst[o++] = table[c & 0x3f];
        }
      else
        {
          dst[o++] = '=';
        }
    }

  if (o < dst_len)
    {
      dst[o] = '\0';
    }
}

static int parse_ws_url(const char *url, struct ws_url_s *out)
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

  if (strncmp(url, "ws://", 5) == 0)
    {
      out->port = 80;
      out->tls = false;
      host_start = url + 5;
    }
  else if (strncmp(url, "wss://", 6) == 0)
    {
      out->port = 443;
      out->tls = true;
      host_start = url + 6;
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

#ifdef CONFIG_CRYPTO_MBEDTLS

/* Cached DNS results for dashscope host, resolved in main thread */

#define WS_DNS_MAX_ADDRS 8
static struct sockaddr_in g_ws_cached_addrs[WS_DNS_MAX_ADDRS];
static int g_ws_cached_count = 0;

/* Call from main thread before WebSocket thread starts */

void bailian_ws_dns_prefetch(const char *host, uint16_t port)
{
  struct addrinfo hints;
  struct addrinfo *res = NULL;
  struct addrinfo *it;
  int ret;
  char port_str[8];

  snprintf(port_str, sizeof(port_str), "%u", port);
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;


  ret = getaddrinfo(host, port_str, &hints, &res);
  if (ret == 0 && res != NULL)
    {
      g_ws_cached_count = 0;
      for (it = res; it != NULL && g_ws_cached_count < WS_DNS_MAX_ADDRS;
           it = it->ai_next)
        {
          if (it->ai_family == AF_INET)
            {
              memcpy(&g_ws_cached_addrs[g_ws_cached_count],
                     it->ai_addr, sizeof(struct sockaddr_in));
              g_ws_cached_count++;
            }
        }

      freeaddrinfo(res);
    }
  else
    {
      printf("[DNS] Pre-resolve failed: %d\n", ret);
      fflush(stdout);
    }
}

/* Helper struct and thread function for connect with timeout.
 * NuttX connect() blocks even with O_NONBLOCK/SOCK_NONBLOCK set
 * (FDCHECK encoded fds break non-blocking connect).
 * Use a helper thread to do blocking connect, with polling
 * sem_trywait for timeout (sem_timedwait may also block on NuttX).
 */

struct connect_helper_s
{
  int fd;
  struct sockaddr_in addr;
  volatile int result;
  volatile int err;
  volatile bool done;
};

static void *connect_thread_func(void *arg)
{
  struct connect_helper_s *h = (struct connect_helper_s *)arg;

  h->result = connect(h->fd, (struct sockaddr *)&h->addr, sizeof(h->addr));
  h->err = errno;
  h->done = true;
  return NULL;
}

/* Force IPv4 connect using cached DNS, try all addresses.
 * Uses a helper thread + polling for connect timeout,
 * because NuttX connect() ignores O_NONBLOCK with FDCHECK.
 */

static int tls_tcp_connect_ipv4(const char *host, uint16_t port,
                                int timeout_ms)
{
  int fd;
  int i;

  if (g_ws_cached_count <= 0)
    {
      printf("[TLS] No cached DNS for %s\n", host);
      return -EHOSTUNREACH;
    }

  for (i = 0; i < g_ws_cached_count; i++)
    {
      struct connect_helper_s helper;
      pthread_t tid;
      int ret;
      int waited_ms;

      memcpy(&helper.addr, &g_ws_cached_addrs[i], sizeof(helper.addr));
      helper.addr.sin_port = htons(port);
      helper.result = -1;
      helper.err = 0;
      helper.done = false;


      fd = socket(AF_INET, SOCK_STREAM, 0);
      if (fd < 0)
        {
          printf("[TLS] socket failed: %d\n", errno);
          continue;
        }


      helper.fd = fd;

      /* Create detached thread to do blocking connect */

      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_setstacksize(&attr, 2048);

      ret = pthread_create(&tid, &attr, connect_thread_func, &helper);
      pthread_attr_destroy(&attr);
      if (ret != 0)
        {
          printf("[TLS] pthread_create failed: %d\n", ret);
          close(fd);
          continue;
        }

      /* Poll for completion with timeout */


      waited_ms = 0;
      while (!helper.done && waited_ms < timeout_ms)
        {
          usleep(50000); /* 50ms intervals */
          waited_ms += 50;
        }

      if (!helper.done)
        {
          /* Timeout - close fd to unblock the connect thread */

          printf("[TLS] connect timeout on IP #%d after %dms\n",
                 i, waited_ms);
          fflush(stdout);
          close(fd);

          /* Wait a bit for thread to notice the closed fd */

          int extra = 0;
          while (!helper.done && extra < 2000)
            {
              usleep(50000);
              extra += 50;
            }

          if (helper.done)
            {
              pthread_join(tid, NULL);
            }
          else
            {
              /* Thread still stuck, detach it to avoid leak */

              printf("[TLS] connect thread still stuck, detaching\n");
              fflush(stdout);
              pthread_detach(tid);
            }

          continue;
        }

      /* Thread finished - join and check result */

      pthread_join(tid, NULL);

      if (helper.result != 0)
        {
          printf("[TLS] connect failed on IP #%d: errno=%d\n",
                 i, helper.err);
          close(fd);
          continue;
        }

      goto connected;
    }

  printf("[TLS] All %d IPs failed\n", g_ws_cached_count);
  return -EHOSTUNREACH;

connected:
  {
    struct timeval tv;

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
  }

  return fd;
}

/* Custom recv with poll() timeout for NuttX compatibility.
 * Cannot use select() because FDCHECK encodes fd values beyond FD_SETSIZE,
 * causing FD_SET() buffer overflow. poll() has no such limitation.
 */

/* Custom send with poll() + detailed logging for NuttX FDCHECK compatibility.
 * Replaces mbedtls_net_send to ensure write() actually delivers data
 * and to diagnose potential FDCHECK issues on the send path.
 */

static int my_net_send(void *ctx, const unsigned char *buf, size_t len)
{
  mbedtls_net_context *net_ctx = (mbedtls_net_context *)ctx;
  struct pollfd pfd;
  int ret;

  pfd.fd = net_ctx->fd;
  pfd.events = POLLOUT;
  pfd.revents = 0;

  ret = poll(&pfd, 1, 10000); /* 10 second timeout */

  if (ret == 0)
    {

    }

  if (ret < 0)
    {
      printf("[TLS-SEND] poll error=%d, fd=%d\n", errno, net_ctx->fd);
      return MBEDTLS_ERR_NET_SEND_FAILED;
    }

  if (pfd.revents & (POLLERR | POLLHUP))
    {
      printf("[TLS-SEND] poll POLLERR/HUP revents=0x%x, fd=%d\n",
             pfd.revents, net_ctx->fd);
      return MBEDTLS_ERR_NET_CONN_RESET;
    }

  ret = (int)write(net_ctx->fd, buf, len);


  if (ret < 0)
    {
      printf("[TLS-SEND] write errno=%d\n", errno);
      if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        {
          return MBEDTLS_ERR_SSL_WANT_WRITE;
        }

      if (errno == EPIPE || errno == ECONNRESET)
        {
          return MBEDTLS_ERR_NET_CONN_RESET;
        }

      return MBEDTLS_ERR_NET_SEND_FAILED;
    }

  return ret;
}

static int my_net_recv(void *ctx, unsigned char *buf, size_t len)
{
  mbedtls_net_context *net_ctx = (mbedtls_net_context *)ctx;
  struct pollfd pfd;
  int ret;

  pfd.fd = net_ctx->fd;
  pfd.events = POLLIN;
  pfd.revents = 0;


  ret = poll(&pfd, 1, 10000); /* 10 second timeout */

  if (ret == 0)
    {

    }

  if (ret < 0)
    {
      printf("[TLS-RECV] poll error=%d, fd=%d\n", errno, net_ctx->fd);
      return MBEDTLS_ERR_NET_RECV_FAILED;
    }

  if (pfd.revents & (POLLERR | POLLHUP))
    {
      printf("[TLS-RECV] poll POLLERR/HUP revents=0x%x, fd=%d\n",
             pfd.revents, net_ctx->fd);
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

  if (ret == 0)
    {
      printf("[TLS-RECV] read returned 0 (connection closed by peer)\n");
    }

  return ret;
}

static int tls_connect(struct ws_tls_s *tls, const char *host,
                       uint16_t port, int timeout_ms)
{
  int ret;
  int fd;

  mbedtls_net_init(&tls->net);
  mbedtls_ssl_init(&tls->ssl);
  mbedtls_ssl_config_init(&tls->conf);
  mbedtls_ctr_drbg_init(&tls->ctr_drbg);
  mbedtls_entropy_init(&tls->entropy);
  mbedtls_x509_crt_init(&tls->cacert);

  /* Load GlobalSign Root CA - R3 certificate */

  ret = mbedtls_x509_crt_parse(&tls->cacert,
                                (const unsigned char *)globalsign_root_r3_pem,
                                sizeof(globalsign_root_r3_pem));
  if (ret != 0)
    {
      printf("[TLS] CA cert parse failed: -0x%x\n", -ret);
      return -EIO;
    }



  ret = mbedtls_ctr_drbg_seed(&tls->ctr_drbg, mbedtls_entropy_func,
                              &tls->entropy, (const unsigned char *)"bailian",
                              strlen("bailian"));
  if (ret != 0)
    {
      return -EIO;
    }



  /* Use custom IPv4-only connect instead of mbedtls_net_connect */
  fd = tls_tcp_connect_ipv4(host, port, timeout_ms);
  if (fd < 0)
    {
      printf("[TLS] TCP connect failed: %d\n", fd);
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
  #ifdef CONFIG_MBEDTLS_DEBUG_C
  mbedtls_ssl_conf_dbg(&tls->conf, my_debug, NULL);
  mbedtls_debug_set_threshold(4);
#endif

  /* Configure CA certificate chain and set verification to OPTIONAL
   * so we can see verification result but still connect if it fails
   */

  mbedtls_ssl_conf_ca_chain(&tls->conf, &tls->cacert, NULL);
  mbedtls_ssl_conf_authmode(&tls->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);


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

  mbedtls_ssl_set_bio(&tls->ssl, &tls->net, my_net_send,
                      my_net_recv, NULL);

  int hs_count = 0;
  do
    {

      ret = mbedtls_ssl_handshake(&tls->ssl);

      hs_count++;
    }
  while (ret == MBEDTLS_ERR_SSL_WANT_READ ||
         ret == MBEDTLS_ERR_SSL_WANT_WRITE);

  if (ret != 0)
    {
      printf("[TLS] Handshake failed: -0x%x\n", -ret);

      /* Print verification result if available */

      uint32_t flags = mbedtls_ssl_get_verify_result(&tls->ssl);
      if (flags != 0)
        {
          char vrfy_buf[256];
          mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf),
                                       "  ! ", flags);
          printf("[TLS] Verify result: %s\n", vrfy_buf);
        }

      fflush(stdout);
      return -EIO;
    }

  /* Print verification result on success too */

  {
    uint32_t flags = mbedtls_ssl_get_verify_result(&tls->ssl);
    if (flags != 0)
      {
        char vrfy_buf[256];
        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf),
                                     "  ! ", flags);
        printf("[TLS] WARNING verify: %s\n", vrfy_buf);
      }
    else
      {
        printf("[TLS] Server certificate verified OK\n");
      }
    
    fflush(stdout);
  }

  tls->inited = true;
  return 0;
}

static void tls_close(struct ws_tls_s *tls)
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
  mbedtls_x509_crt_free(&tls->cacert);
  tls->inited = false;
}

static ssize_t tls_write(struct ws_tls_s *tls, const uint8_t *data, size_t len)
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

static ssize_t tls_read(struct ws_tls_s *tls, uint8_t *data, size_t len)
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

static ssize_t ws_write(struct bailian_ws_client *client, const uint8_t *data,
                        size_t len)
{
  if (client->tls)
    {
#ifdef CONFIG_CRYPTO_MBEDTLS
      return tls_write(&client->tls_ctx, data, len);
#else
      return -ENOTSUP;
#endif
    }

  return send(client->fd, data, len, 0);
}

/* Single read attempt — may return fewer bytes than requested */
static ssize_t ws_read_once(struct bailian_ws_client *client, uint8_t *data,
                            size_t len, int timeout_ms)
{
  if (client->tls)
    {
#ifdef CONFIG_CRYPTO_MBEDTLS
      return tls_read(&client->tls_ctx, data, len);
#else
      return -ENOTSUP;
#endif
    }
  if (timeout_ms > 0)
    {
      struct pollfd pfd;
      pfd.fd = client->fd;
      pfd.events = POLLIN;
      pfd.revents = 0;
      if (poll(&pfd, 1, timeout_ms) <= 0)
        {
          return 0;
        }
    }
  return recv(client->fd, data, len, 0);
}

/* Read exactly 'len' bytes, looping for partial TLS reads.
 * mbedtls_ssl_read may return fewer bytes than requested
 * (e.g. request 8000, get 4096 from one TLS record).
 * Returns len on success, <= 0 on error/timeout. */
static ssize_t ws_read_exact(struct bailian_ws_client *client, uint8_t *data,
                             size_t len, int timeout_ms)
{
  size_t total = 0;
  while (total < len)
    {
      ssize_t ret = ws_read_once(client, data + total, len - total, timeout_ms);
      if (ret < 0)
        {
          return ret;
        }
      if (ret == 0)
        {
          if (total == 0) return 0;  /* timeout before any data */
          /* Got partial data, keep trying */
          usleep(1000);
          continue;
        }
      total += (size_t)ret;
    }
  return (ssize_t)total;
}

static int ws_build_accept(const char *key, char *accept, size_t accept_len)
{
  char concat[128];
  uint8_t digest[20];
  sha1_ctx_t ctx;

  if (snprintf(concat, sizeof(concat), "%s%s", key, WS_GUID) < 0)
    {
      return -EINVAL;
    }

  sha1_init(&ctx);
  sha1_update(&ctx, (const uint8_t *)concat, strlen(concat));
  sha1_final(&ctx, digest);
  base64_encode(digest, sizeof(digest), accept, accept_len);
  return 0;
}

void bailian_ws_close(struct bailian_ws_client *client);

int bailian_ws_connect(const char *url, const char *extra_headers,
                       int timeout_ms, struct bailian_ws_client **out)
{
  struct ws_url_s parsed;
  struct bailian_ws_client *client;
  uint8_t key_raw[16];
  char key_b64[64];
  char accept[64];
  char request[512];
  char response[512];
  ssize_t ret;

  if (parse_ws_url(url, &parsed) != 0 || out == NULL)
    {
      printf("[WS] URL parse failed\n");
      return -EINVAL;
    }

  client = calloc(1, sizeof(*client));
  if (client == NULL)
    {
      printf("[WS] calloc failed\n");
      return -ENOMEM;
    }

  client->tls = parsed.tls;

  if (parsed.tls)
    {
#ifdef CONFIG_CRYPTO_MBEDTLS
      ret = tls_connect(&client->tls_ctx, parsed.host, parsed.port, timeout_ms);
      if (ret < 0)
        {
          free(client);
          return (int)ret;
        }
#else
      free(client);
      return -ENOTSUP;
#endif
    }
  else
    {
      client->fd = tcp_connect(parsed.host, parsed.port, timeout_ms);
      if (client->fd < 0)
        {
          free(client);
          return client->fd;
        }
    }

  for (size_t i = 0; i < sizeof(key_raw); i++)
    {
      key_raw[i] = (uint8_t)(rand() & 0xff);
    }
  base64_encode(key_raw, sizeof(key_raw), key_b64, sizeof(key_b64));
  ws_build_accept(key_b64, accept, sizeof(accept));

  snprintf(request, sizeof(request),
           "GET %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Upgrade: websocket\r\n"
           "Connection: Upgrade\r\n"
           "Sec-WebSocket-Key: %s\r\n"
           "Sec-WebSocket-Version: 13\r\n"
           "%s\r\n",
           parsed.path, parsed.host, key_b64,
           extra_headers != NULL ? extra_headers : "");

  ret = ws_write(client, (const uint8_t *)request, strlen(request));
  if (ret < 0)
    {
      bailian_ws_close(client);
      return (int)ret;
    }

  ret = ws_read_once(client, (uint8_t *)response, sizeof(response) - 1, timeout_ms);
  if (ret <= 0)
    {
      bailian_ws_close(client);
      return -EIO;
    }

  response[ret] = '\0';
  if (strstr(response, " 101 ") == NULL ||
      strcasestr(response, "Sec-WebSocket-Accept") == NULL ||
      strstr(response, accept) == NULL)
    {
      bailian_ws_close(client);
      return -EPROTO;
    }

  *out = client;
  return 0;
}

int bailian_ws_send(struct bailian_ws_client *client, uint8_t opcode,
                    const uint8_t *data, size_t len)
{
  uint8_t header[14];
  uint8_t mask[4];
  size_t header_len = 0;
  size_t i;

  if (client == NULL)
    {
      return -EINVAL;
    }

  header[0] = 0x80 | (opcode & 0x0f);
  if (len < 126)
    {
      header[1] = 0x80 | (uint8_t)len;
      header_len = 2;
    }
  else if (len < 65536)
    {
      header[1] = 0x80 | 126;
      header[2] = (uint8_t)(len >> 8);
      header[3] = (uint8_t)(len & 0xff);
      header_len = 4;
    }
  else
    {
      header[1] = 0x80 | 127;
      for (i = 0; i < 8; i++)
        {
          header[2 + i] = (uint8_t)((len >> ((7 - i) * 8)) & 0xff);
        }
      header_len = 10;
    }

  for (i = 0; i < 4; i++)
    {
      mask[i] = (uint8_t)(rand() & 0xff);
    }

  memcpy(&header[header_len], mask, 4);
  header_len += 4;

  if (ws_write(client, header, header_len) < 0)
    {
      return -EIO;
    }

  if (len > 0)
    {
      uint8_t *masked = malloc(len);
      if (masked == NULL)
        {
          return -ENOMEM;
        }
      for (i = 0; i < len; i++)
        {
          masked[i] = data[i] ^ mask[i % 4];
        }
      if (ws_write(client, masked, len) < 0)
        {
          free(masked);
          return -EIO;
        }
      free(masked);
    }

  return 0;
}

int bailian_ws_recv(struct bailian_ws_client *client, uint8_t *opcode,
                    uint8_t *data, size_t len, int timeout_ms)
{
  uint8_t header[2];
  uint8_t ext_len[8];
  size_t payload_len;
  int masked;
  ssize_t ret;

  if (client == NULL || opcode == NULL || data == NULL)
    {
      return -EINVAL;
    }

  ret = ws_read_once(client, header, sizeof(header), timeout_ms);
  if (ret <= 0)
    {
      return (int)ret;
    }

  *opcode = header[0] & 0x0f;
  masked = (header[1] & 0x80) != 0;
  payload_len = (header[1] & 0x7f);

  if (payload_len == 126)
    {
      ret = ws_read_exact(client, ext_len, 2, timeout_ms);
      if (ret != 2)
        {
          return -EIO;
        }
      payload_len = ((size_t)ext_len[0] << 8) | ext_len[1];
    }
  else if (payload_len == 127)
    {
      ret = ws_read_exact(client, ext_len, 8, timeout_ms);
      if (ret != 8)
        {
          return -EIO;
        }
      payload_len = 0;
      for (int i = 0; i < 8; i++)
        {
          payload_len = (payload_len << 8) | ext_len[i];
        }
    }

  if (payload_len > len)
    {
      /* Frame too large for buffer — drain it to keep stream in sync */
      printf("[WS] frame too large: %zu > %zu, draining\n",
             payload_len, len);
      size_t skip = payload_len + (masked ? 4 : 0);
      uint8_t drain[512];
      while (skip > 0)
        {
          size_t chunk = skip < sizeof(drain) ? skip : sizeof(drain);
          ret = ws_read_once(client, drain, chunk, timeout_ms);
          if (ret <= 0)
            {
              return -EIO;  /* Connection broken during drain */
            }
          skip -= (size_t)ret;
        }
      return 0;  /* Frame skipped, not an error */
    }

  if (masked)
    {
      uint8_t mask[4];
      ret = ws_read_exact(client, mask, sizeof(mask), timeout_ms);
      if (ret != 4)
        {
          return -EIO;
        }

      ret = ws_read_exact(client, data, payload_len, timeout_ms);
      if (ret != (ssize_t)payload_len)
        {
          return -EIO;
        }

      for (size_t i = 0; i < payload_len; i++)
        {
          data[i] ^= mask[i % 4];
        }
    }
  else
    {
      ret = ws_read_exact(client, data, payload_len, timeout_ms);
      if (ret != (ssize_t)payload_len)
        {
          return -EIO;
        }
    }

  if (*opcode == 0x9)
    {
      bailian_ws_send(client, 0xA, data, payload_len);
      return 0;
    }

  return (int)payload_len;
}

void bailian_ws_close(struct bailian_ws_client *client)
{
  if (client == NULL)
    {
      return;
    }

  if (client->tls)
    {
#ifdef CONFIG_CRYPTO_MBEDTLS
      tls_close(&client->tls_ctx);
#endif
    }
  else if (client->fd >= 0)
    {
      close(client->fd);
    }

  free(client);
}
