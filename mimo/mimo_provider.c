/****************************************************************************
 * packages/demos/mimo/mimo_provider.c
 *
 * Copyright (C) 2024 Xiaomi Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netutils/cJSON.h>
#include <netutils/webclient.h>

#ifdef CONFIG_CRYPTO_MBEDTLS
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <unistd.h>
#  include <mbedtls/ssl.h>
#  include <mbedtls/entropy.h>
#  include <mbedtls/ctr_drbg.h>
#endif

#include "mimo.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/

static char g_api_key[256];
static char g_base_url[MIMO_MAX_URL_LEN];

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* Accumulator for HTTP response body received in chunks */

struct http_resp_buf_s
{
  char  *data;
  size_t len;
  size_t cap;
};

#ifdef CONFIG_CRYPTO_MBEDTLS

/* TLS context wrapping mbedtls state */

struct mimo_tls_ctx_s
{
  mbedtls_entropy_context  entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_ssl_config       conf;
};

/* TLS connection wrapping a single mbedtls session.
 * webclient_tls_connection is opaque (forward-declared only),
 * so we cast our struct pointer to/from it as an opaque handle.
 * We use a plain POSIX fd instead of mbedtls_net_context to
 * avoid mbedtls_net_connect's getaddrinfo issues on NuttX.
 */

struct mimo_tls_conn_s
{
  mbedtls_ssl_context  ssl;
  int                  fd;
};
#endif

/****************************************************************************
 * Private Functions - TLS
 ****************************************************************************/

#ifdef CONFIG_CRYPTO_MBEDTLS

/* BIO callbacks for mbedtls using a plain fd */

static int mimo_bio_send(void *ctx, const unsigned char *buf, size_t len)
{
  int fd = *(int *)ctx;
  ssize_t ret = send(fd, buf, len, 0);
  if (ret < 0)
    {
      return -EIO;
    }

  return (int)ret;
}

static int mimo_bio_recv(void *ctx, unsigned char *buf, size_t len)
{
  int fd = *(int *)ctx;
  ssize_t ret = recv(fd, buf, len, 0);
  if (ret < 0)
    {
      return -EIO;
    }

  if (ret == 0)
    {
      return MBEDTLS_ERR_SSL_WANT_READ;
    }

  return (int)ret;
}

static int mimo_tls_connect(FAR void *ctx,
                            FAR const char *hostname,
                            FAR const char *port,
                            unsigned int timeout_sec,
                            FAR struct webclient_tls_connection **connp)
{
  FAR struct mimo_tls_ctx_s *tctx = ctx;
  FAR struct mimo_tls_conn_s *conn;
  struct hostent *he;
  struct sockaddr_in server;
  int portnum;
  int ret;
  int fd;

  printf("[mimo] TLS connecting to %s:%s\n", hostname, port);

  /* Resolve hostname using gethostbyname (more reliable on NuttX) */

  he = gethostbyname(hostname);
  if (!he)
    {
      fprintf(stderr, "[mimo] DNS resolve failed for %s\n", hostname);
      return -EIO;
    }

  /* Create TCP socket and connect */

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
    {
      fprintf(stderr, "[mimo] socket() failed: %d\n", errno);
      return -EIO;
    }

  portnum = atoi(port);
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(portnum);
  memcpy(&server.sin_addr, he->h_addr_list[0], he->h_length);

  printf("[mimo] Connecting to %s (%s:%d)\n",
         hostname, inet_ntoa(server.sin_addr), portnum);

  if (connect(fd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
      fprintf(stderr, "[mimo] TCP connect failed: %d\n", errno);
      close(fd);
      return -EIO;
    }

  printf("[mimo] TCP connected, starting TLS handshake\n");

  /* Allocate TLS connection wrapper */

  conn = calloc(1, sizeof(*conn));
  if (!conn)
    {
      close(fd);
      return -ENOMEM;
    }

  conn->fd = fd;
  mbedtls_ssl_init(&conn->ssl);

  ret = mbedtls_ssl_setup(&conn->ssl, &tctx->conf);
  if (ret != 0)
    {
      fprintf(stderr, "[mimo] TLS ssl setup failed: -0x%x\n", -ret);
      goto err;
    }

  ret = mbedtls_ssl_set_hostname(&conn->ssl, hostname);
  if (ret != 0)
    {
      fprintf(stderr, "[mimo] TLS set hostname failed: -0x%x\n", -ret);
      goto err;
    }

  mbedtls_ssl_set_bio(&conn->ssl, &conn->fd,
                       mimo_bio_send, mimo_bio_recv, NULL);

  while ((ret = mbedtls_ssl_handshake(&conn->ssl)) != 0)
    {
      if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
          ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
          fprintf(stderr, "[mimo] TLS handshake failed: -0x%x\n", -ret);
          goto err;
        }
    }

  printf("[mimo] TLS handshake complete\n");

  *connp = (FAR struct webclient_tls_connection *)conn;
  return 0;

err:
  mbedtls_ssl_free(&conn->ssl);
  close(conn->fd);
  free(conn);
  return -EIO;
}

static ssize_t mimo_tls_send(FAR void *ctx,
                             FAR struct webclient_tls_connection *base,
                             FAR const void *buf, size_t len)
{
  FAR struct mimo_tls_conn_s *conn = (FAR struct mimo_tls_conn_s *)base;
  int ret;

  (void)ctx;
  ret = mbedtls_ssl_write(&conn->ssl, buf, len);
  if (ret < 0)
    {
      if (ret == MBEDTLS_ERR_SSL_WANT_WRITE)
        {
          return 0;
        }

      return -EIO;
    }

  return ret;
}

static ssize_t mimo_tls_recv(FAR void *ctx,
                             FAR struct webclient_tls_connection *base,
                             FAR void *buf, size_t len)
{
  FAR struct mimo_tls_conn_s *conn = (FAR struct mimo_tls_conn_s *)base;
  int ret;

  (void)ctx;
  ret = mbedtls_ssl_read(&conn->ssl, buf, len);
  if (ret < 0)
    {
      if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
        {
          return 0;
        }

      if (ret == MBEDTLS_ERR_SSL_WANT_READ)
        {
          return 0;
        }

      return -EIO;
    }

  return ret;
}

static int mimo_tls_close(FAR void *ctx,
                          FAR struct webclient_tls_connection *base)
{
  FAR struct mimo_tls_conn_s *conn = (FAR struct mimo_tls_conn_s *)base;

  (void)ctx;
  mbedtls_ssl_close_notify(&conn->ssl);
  mbedtls_ssl_free(&conn->ssl);
  close(conn->fd);
  free(conn);
  return 0;
}

static int mimo_tls_get_poll_info(
    FAR void *ctx,
    FAR struct webclient_tls_connection *base,
    FAR struct webclient_poll_info *info)
{
  FAR struct mimo_tls_conn_s *conn = (FAR struct mimo_tls_conn_s *)base;

  (void)ctx;
  info->fd = conn->fd;
  info->flags = WEBCLIENT_POLL_INFO_WANT_READ;
  return 0;
}

static const struct webclient_tls_ops g_mimo_tls_ops =
{
  .connect        = mimo_tls_connect,
  .send           = mimo_tls_send,
  .recv           = mimo_tls_recv,
  .close          = mimo_tls_close,
  .get_poll_info  = mimo_tls_get_poll_info,
  .init_connection = NULL,
};

static struct mimo_tls_ctx_s g_tls_ctx;
static bool g_tls_initialized = false;

static int mimo_tls_init(void)
{
  int ret;

  if (g_tls_initialized)
    {
      return 0;
    }

  mbedtls_entropy_init(&g_tls_ctx.entropy);
  mbedtls_ctr_drbg_init(&g_tls_ctx.ctr_drbg);
  mbedtls_ssl_config_init(&g_tls_ctx.conf);

  ret = mbedtls_ctr_drbg_seed(&g_tls_ctx.ctr_drbg,
                               mbedtls_entropy_func,
                               &g_tls_ctx.entropy,
                               (const unsigned char *)"mimo", 4);
  if (ret != 0)
    {
      fprintf(stderr, "[mimo] TLS drbg seed failed: -0x%x\n", -ret);
      return -1;
    }

  ret = mbedtls_ssl_config_defaults(&g_tls_ctx.conf,
                                     MBEDTLS_SSL_IS_CLIENT,
                                     MBEDTLS_SSL_TRANSPORT_STREAM,
                                     MBEDTLS_SSL_PRESET_DEFAULT);
  if (ret != 0)
    {
      fprintf(stderr, "[mimo] TLS config defaults failed: -0x%x\n", -ret);
      return -1;
    }

  /* Skip server certificate verification for embedded use.
   * For production, configure CA certificates here.
   */

  mbedtls_ssl_conf_authmode(&g_tls_ctx.conf, MBEDTLS_SSL_VERIFY_NONE);
  mbedtls_ssl_conf_rng(&g_tls_ctx.conf, mbedtls_ctr_drbg_random,
                        &g_tls_ctx.ctr_drbg);

  g_tls_initialized = true;
  return 0;
}

static void mimo_tls_deinit(void)
{
  if (!g_tls_initialized)
    {
      return;
    }

  mbedtls_ssl_config_free(&g_tls_ctx.conf);
  mbedtls_ctr_drbg_free(&g_tls_ctx.ctr_drbg);
  mbedtls_entropy_free(&g_tls_ctx.entropy);
  g_tls_initialized = false;
}
#endif /* CONFIG_CRYPTO_MBEDTLS */

/****************************************************************************
 * Private Functions - HTTP
 ****************************************************************************/

/**
 * Sink callback for webclient: accumulates response body chunks.
 */

static int http_sink_callback(FAR char **buffer, int offset,
                              int datend, FAR int *buflen,
                              FAR void *arg)
{
  FAR struct http_resp_buf_s *resp = arg;
  int len = datend - offset;

  if (len <= 0)
    {
      return 0;
    }

  /* Grow buffer if needed */

  while (resp->len + len + 1 > resp->cap)
    {
      size_t newcap = resp->cap ? resp->cap * 2 : 4096;
      char *newdata = realloc(resp->data, newcap);
      if (!newdata)
        {
          fprintf(stderr, "[mimo] OOM in sink callback\n");
          return -ENOMEM;
        }

      resp->data = newdata;
      resp->cap = newcap;
    }

  memcpy(resp->data + resp->len, &((*buffer)[offset]), len);
  resp->len += len;
  resp->data[resp->len] = '\0';
  return 0;
}

/****************************************************************************
 * Private Functions - Request Builder
 ****************************************************************************/

/**
 * Build MiMo request body (OpenAI chat completions + MiMo extensions).
 */

static cJSON *build_request_body(const struct mimo_chat_req_s *req)
{
  cJSON *root = cJSON_CreateObject();
  cJSON *messages;
  int i;

  cJSON_AddStringToObject(root, "model", req->model);

  /* MiMo recommended: top_p=0.95 */

  cJSON_AddNumberToObject(root, "top_p", 0.95);

  if (req->temperature > 0)
    {
      cJSON_AddNumberToObject(root, "temperature",
                              (double)req->temperature / 100.0);
    }

  cJSON_AddNumberToObject(root, "max_tokens", 8192);

  /* Enable thinking mode if configured */

#ifdef CONFIG_MIMO_ENABLE_THINKING
  {
    cJSON *kwargs = cJSON_CreateObject();
    cJSON_AddTrueToObject(kwargs, "enable_thinking");
    cJSON_AddItemToObject(root, "chat_template_kwargs", kwargs);
  }
#endif

  /* Build messages array */

  messages = cJSON_AddArrayToObject(root, "messages");

  /* System prompt */

  if (req->system_prompt && strlen(req->system_prompt) > 0)
    {
      cJSON *sys = cJSON_CreateObject();
      cJSON_AddStringToObject(sys, "role", "system");
      cJSON_AddStringToObject(sys, "content", req->system_prompt);
      cJSON_AddItemToArray(messages, sys);
    }

  /* Conversation messages */

  for (i = 0; i < req->message_count; i++)
    {
      cJSON *msg = cJSON_CreateObject();
      const char *role_str;

      switch (req->messages[i].role)
        {
          case MIMO_ROLE_USER:
            role_str = "user";
            break;
          case MIMO_ROLE_ASSISTANT:
            role_str = "assistant";
            break;
          case MIMO_ROLE_TOOL:
            role_str = "tool";
            break;
          default:
            role_str = "user";
            break;
        }

      cJSON_AddStringToObject(msg, "role", role_str);

      if (req->messages[i].content)
        {
          cJSON_AddStringToObject(msg, "content",
                                  req->messages[i].content);
        }

      if (req->messages[i].tool_call_id)
        {
          cJSON_AddStringToObject(msg, "tool_call_id",
                                  req->messages[i].tool_call_id);
        }

      /* Preserve reasoning_content for multi-turn tool calls
       * in thinking mode (MiMo requirement: all history
       * reasoning_content must be sent back).
       */

      if (req->messages[i].role == MIMO_ROLE_ASSISTANT &&
          req->messages[i].reasoning_content)
        {
          cJSON_AddStringToObject(msg, "reasoning_content",
                                  req->messages[i].reasoning_content);
        }

      cJSON_AddItemToArray(messages, msg);
    }

  /* Tools (OpenAI function calling format) */

#ifdef CONFIG_MIMO_ENABLE_TOOLS
  if (req->tools_json && strlen(req->tools_json) > 0)
    {
      cJSON *tools = cJSON_Parse(req->tools_json);
      if (tools)
        {
          cJSON_AddItemToObject(root, "tools", tools);
        }
    }
#endif

  return root;
}

/**
 * Parse MiMo response.
 *
 * Handles standard OpenAI fields plus MiMo's reasoning_content
 * extension returned alongside tool_calls in thinking mode.
 */

static int parse_response(const char *json_str,
                          struct mimo_chat_resp_s *resp)
{
  cJSON *root;
  cJSON *choices;
  cJSON *choice;
  cJSON *message;
  cJSON *content;
  cJSON *tool_calls;

  memset(resp, 0, sizeof(*resp));

  root = cJSON_Parse(json_str);
  if (!root)
    {
      fprintf(stderr, "[mimo] JSON parse error\n");
      return -1;
    }

  /* Check for API error */

  cJSON *error = cJSON_GetObjectItem(root, "error");
  if (error)
    {
      cJSON *errmsg = cJSON_GetObjectItem(error, "message");
      fprintf(stderr, "[mimo] API error: %s\n",
              errmsg ? errmsg->valuestring : "unknown");
      cJSON_Delete(root);
      return -1;
    }

  choices = cJSON_GetObjectItem(root, "choices");
  if (!choices || cJSON_GetArraySize(choices) == 0)
    {
      fprintf(stderr, "[mimo] No choices in response\n");
      cJSON_Delete(root);
      return -1;
    }

  choice = cJSON_GetArrayItem(choices, 0);
  message = cJSON_GetObjectItem(choice, "message");
  if (!message)
    {
      cJSON_Delete(root);
      return -1;
    }

  /* Extract reasoning_content (MiMo thinking mode) */

  cJSON *reasoning = cJSON_GetObjectItem(message, "reasoning_content");
  if (reasoning && reasoning->valuestring)
    {
      resp->reasoning_content = strdup(reasoning->valuestring);
      printf("[mimo] Thinking: %.80s%s\n",
             reasoning->valuestring,
             strlen(reasoning->valuestring) > 80 ? "..." : "");
    }

  /* Extract content */

  content = cJSON_GetObjectItem(message, "content");
  if (content && content->valuestring)
    {
      resp->content = strdup(content->valuestring);
    }

  /* Check for tool calls (OpenAI format) */

  tool_calls = cJSON_GetObjectItem(message, "tool_calls");
  if (tool_calls && cJSON_GetArraySize(tool_calls) > 0)
    {
      cJSON *tc = cJSON_GetArrayItem(tool_calls, 0);
      cJSON *tc_id = cJSON_GetObjectItem(tc, "id");
      cJSON *func = cJSON_GetObjectItem(tc, "function");

      if (func)
        {
          cJSON *fname = cJSON_GetObjectItem(func, "name");
          cJSON *fargs = cJSON_GetObjectItem(func, "arguments");

          resp->is_tool_use = true;

          if (tc_id && tc_id->valuestring)
            {
              resp->tool_call_id = strdup(tc_id->valuestring);
            }

          if (fname && fname->valuestring)
            {
              resp->tool_name = strdup(fname->valuestring);
            }

          if (fargs && fargs->valuestring)
            {
              resp->tool_input = strdup(fargs->valuestring);
            }
        }
    }

  cJSON_Delete(root);
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int mimo_provider_init(const char *api_key, const char *base_url)
{
  if (!api_key || strlen(api_key) == 0)
    {
      fprintf(stderr, "[mimo] API key not configured\n");
      return -1;
    }

  strncpy(g_api_key, api_key, sizeof(g_api_key) - 1);
  strncpy(g_base_url, base_url, sizeof(g_base_url) - 1);

  printf("[mimo] Provider initialized\n");
  printf("[mimo] Endpoint: %s\n", g_base_url);
  printf("[mimo] Thinking mode: %s\n",
#ifdef CONFIG_MIMO_ENABLE_THINKING
         "enabled"
#else
         "disabled"
#endif
         );
  return 0;
}

int mimo_provider_chat(const struct mimo_chat_req_s *req,
                       struct mimo_chat_resp_s *resp)
{
  cJSON *body;
  char *body_str;
  char auth_header[300];
  const char *headers[2];
  struct webclient_context ctx;
  struct http_resp_buf_s resp_buf;
  char *work_buf;
  int ret;

  body = build_request_body(req);
  if (!body)
    {
      return -1;
    }

  body_str = cJSON_PrintUnformatted(body);
  cJSON_Delete(body);

  if (!body_str)
    {
      return -1;
    }

  printf("[mimo] POST %s (%d bytes)\n",
         g_base_url, (int)strlen(body_str));

  /* Prepare HTTP headers */

  snprintf(auth_header, sizeof(auth_header),
           "Authorization: Bearer %s", g_api_key);
  headers[0] = "Content-Type: application/json";
  headers[1] = auth_header;

  /* Allocate work buffer for webclient (used for HTTP header I/O) */

  work_buf = malloc(2048);
  if (!work_buf)
    {
      free(body_str);
      return -1;
    }

  /* Initialize response accumulator */

  memset(&resp_buf, 0, sizeof(resp_buf));

  /* Set up webclient context */

  webclient_set_defaults(&ctx);
  ctx.protocol_version = WEBCLIENT_PROTOCOL_VERSION_HTTP_1_1;
  ctx.method   = "POST";
  ctx.url      = g_base_url;
  ctx.buffer   = work_buf;
  ctx.buflen   = 2048;
  ctx.headers  = headers;
  ctx.nheaders = 2;
  ctx.sink_callback     = http_sink_callback;
  ctx.sink_callback_arg = &resp_buf;
  ctx.timeout_sec       = 60;

#ifdef CONFIG_CRYPTO_MBEDTLS
  /* Enable TLS for HTTPS */

  ret = mimo_tls_init();
  if (ret < 0)
    {
      fprintf(stderr, "[mimo] TLS init failed\n");
      free(work_buf);
      free(body_str);
      return -1;
    }

  ctx.tls_ops = &g_mimo_tls_ops;
  ctx.tls_ctx = &g_tls_ctx;
#endif

  webclient_set_static_body(&ctx, body_str, strlen(body_str));

  ret = webclient_perform(&ctx);

  free(work_buf);
  free(body_str);

  if (ret < 0)
    {
      fprintf(stderr, "[mimo] HTTP request failed: %d\n", ret);
      if (resp_buf.data)
        {
          free(resp_buf.data);
        }

      return -1;
    }

  printf("[mimo] HTTP %d, response %zu bytes\n",
         ctx.http_status, resp_buf.len);

  if (ctx.http_status != 200 || !resp_buf.data)
    {
      fprintf(stderr, "[mimo] HTTP error %d: %s\n",
              ctx.http_status,
              resp_buf.data ? resp_buf.data : "(empty)");
      if (resp_buf.data)
        {
          free(resp_buf.data);
        }

      return -1;
    }

  /* Parse the JSON response */

  ret = parse_response(resp_buf.data, resp);
  free(resp_buf.data);

  return ret;
}

void mimo_chat_resp_free(struct mimo_chat_resp_s *resp)
{
  if (resp->content)
    {
      free(resp->content);
      resp->content = NULL;
    }

  if (resp->tool_call_id)
    {
      free(resp->tool_call_id);
      resp->tool_call_id = NULL;
    }

  if (resp->tool_name)
    {
      free(resp->tool_name);
      resp->tool_name = NULL;
    }

  if (resp->tool_input)
    {
      free(resp->tool_input);
      resp->tool_input = NULL;
    }

  if (resp->reasoning_content)
    {
      free(resp->reasoning_content);
      resp->reasoning_content = NULL;
    }
}

void mimo_provider_destroy(void)
{
  memset(g_api_key, 0, sizeof(g_api_key));
#ifdef CONFIG_CRYPTO_MBEDTLS
  mimo_tls_deinit();
#endif
}
