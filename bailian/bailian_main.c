#include <nuttx/config.h>

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "c_mmi.h"
#include "c_mmi_config.h"
#include "c_mmi_msg.h"
#include "c_mmi_storage.h"
/* lib_c_license.h removed - using pay-as-you-go mode */
#include "c_utils/hal_util_mem.h"
#include "c_utils/hal_util_time.h"
#include "c_utils/hal_util_random.h"
#include "c_utils/hal_util_mutex.h"
#include "c_utils/hal_util_storage.h"
#include "qwen_test.h"

#ifndef CONFIG_AI_BAILIAN_WORKSPACE_ID
#define CONFIG_AI_BAILIAN_WORKSPACE_ID ""
#endif

#ifndef CONFIG_AI_BAILIAN_APP_ID
#define CONFIG_AI_BAILIAN_APP_ID ""
#endif

#ifndef CONFIG_AI_BAILIAN_API_KEY
#define CONFIG_AI_BAILIAN_API_KEY ""
#endif

/* License mode configs removed - using pay-as-you-go mode */

#ifndef CONFIG_AI_BAILIAN_RECORDER_DEV
#define CONFIG_AI_BAILIAN_RECORDER_DEV "/dev/pcm_in0"
#endif

#ifndef CONFIG_AI_BAILIAN_PLAYER_DEV
#define CONFIG_AI_BAILIAN_PLAYER_DEV "/dev/pcm_out0"
#endif

struct bailian_recorder;
struct bailian_player;
struct bailian_ws_client;

/* License mode HTTP functions removed - not needed for pay-as-you-go mode */

struct bailian_recorder *bailian_recorder_create(const char *dev,
                                                 int sample_rate,
                                                 int channels,
                                                 int bits_per_sample);
ssize_t bailian_recorder_read(struct bailian_recorder *rec, uint8_t *buffer,
                              size_t len);
void bailian_recorder_destroy(struct bailian_recorder *rec);
int bailian_recorder_open(struct bailian_recorder *rec, const char *dev,
                          int sample_rate, int channels, int bits_per_sample);
void bailian_recorder_close(struct bailian_recorder *rec);

struct bailian_player *bailian_player_create(const char *dev, int sample_rate,
                                             int channels, int bits_per_sample);
struct bailian_player *bailian_player_create_from_file(const char *filepath);
int bailian_player_start(struct bailian_player *player);
int bailian_player_stop(struct bailian_player *player);
int bailian_player_is_stopped(struct bailian_player *player);
ssize_t bailian_player_write(struct bailian_player *player,
                             const uint8_t *buffer, size_t len);
void bailian_player_destroy(struct bailian_player *player);

int bailian_ws_connect(const char *url, const char *extra_headers,
                       int timeout_ms, struct bailian_ws_client **out);
void bailian_ws_dns_prefetch(const char *host, uint16_t port);
int bailian_ws_send(struct bailian_ws_client *client, uint8_t opcode,
                    const uint8_t *data, size_t len);
int bailian_ws_recv(struct bailian_ws_client *client, uint8_t *opcode,
                    uint8_t *data, size_t len, int timeout_ms);
void bailian_ws_close(struct bailian_ws_client *client);

struct bailian_app_s
{
  struct bailian_recorder *recorder;
  struct bailian_player *player;
  pthread_t recorder_thread;
  pthread_t player_thread;
  pthread_t ws_thread;
  volatile bool running;
  volatile bool recording;
  volatile bool playing;
  volatile bool speech_started;  /* SDK event: ready to send audio */
  volatile bool stop_requested;  /* user pressed 's' */
  volatile int64_t record_start_ms; /* timestamp when recording started */

  /* Our own audio buffer — SDK's recorder_rb gets reset internally */
  pthread_mutex_t audio_mutex;
  uint8_t *audio_buf;
  uint32_t audio_buf_size;
  volatile uint32_t audio_write_pos;
  volatile uint32_t audio_read_pos;
  volatile bool audio_done;  /* recorder finished writing */
};

static struct bailian_app_s g_bailian;

static int64_t bailian_now_ms(void)
{
  struct timespec ts;

  if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
    {
      return 0;
    }

  return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static void *bailian_recorder_loop(void *arg)
{
  struct bailian_app_s *app = (struct bailian_app_s *)arg;
  uint8_t buffer[960];
  uint32_t total_bytes = 0;
  uint32_t read_count = 0;

  while (app->running)
    {
      if (!app->recording)
        {
          if (total_bytes > 0)
            {
              printf("[bailian] recorder: sent total %u bytes in %u reads\n",
                     total_bytes, read_count);
              total_bytes = 0;
              read_count = 0;
              app->audio_done = true;
            }
          usleep(10000);
          continue;
        }

      ssize_t ret = bailian_recorder_read(app->recorder, buffer, sizeof(buffer));
      if (ret > 0)
        {
          int16_t *samples = (int16_t *)buffer;
          int nsamples = ret / 2;
          int64_t sum_sq = 0;
          int16_t peak = 0;
          for (int i = 0; i < nsamples; i++)
            {
              int16_t s = samples[i];
              sum_sq += (int64_t)s * s;
              if (s < 0) s = -s;
              if (s > peak) peak = s;
            }
          if (read_count % 50 == 0)
            {
              printf("[bailian] audio: read=%zd rms=%u peak=%d total=%u\n",
                     ret, (uint32_t)(sum_sq / nsamples),
                     (int)peak, total_bytes);
            }
          total_bytes += ret;
          read_count++;
          /* Feed audio to SDK so it can send binary frames via
           * c_mmi_get_send_data() after SpeechStarted */
          c_mmi_put_recorder_data(buffer, ret);
          /* Also copy to our own buffer as backup */
          pthread_mutex_lock(&app->audio_mutex);
          uint32_t wp = app->audio_write_pos;
          if (wp + (uint32_t)ret <= app->audio_buf_size)
            {
              memcpy(app->audio_buf + wp, buffer, ret);
              app->audio_write_pos = wp + (uint32_t)ret;
            }
          pthread_mutex_unlock(&app->audio_mutex);
        }
      else
        {
          usleep(5000);
        }
    }
  return NULL;
}

static void *bailian_player_loop(void *arg)
{
  struct bailian_app_s *app = (struct bailian_app_s *)arg;
  uint8_t buffer[8192];  /* 8KB per official doc recommendation */
  uint8_t upsample_buf[16384]; /* 2x for 24kHz→48kHz upsampling */
  bool player_active = false;

  while (app->running)
    {
      /* Always try to read from player_rb — don't gate on playing flag.
       * SDK writes TTS audio into player_rb from the ws recv path.
       * If we don't consume it, player_rb fills up and SDK blocks,
       * which stalls the ws_loop recv and causes heartbeat timeout. */
      uint32_t got = c_mmi_get_player_data(buffer, sizeof(buffer));
      if (got > 0)
        {
          if (!player_active)
            {
              printf("[PLAYER] TTS audio started, got %u bytes\n",
                     (unsigned)got);
              bailian_player_start(app->player);
              player_active = true;
            }

          /* Upsample 24kHz→48kHz: duplicate each 16-bit sample */
          {
            const int16_t *src = (const int16_t *)buffer;
            int16_t *dst = (int16_t *)upsample_buf;
            uint32_t nsamples = got / 2;
            uint32_t i;
            for (i = 0; i < nsamples; i++)
              {
                dst[i * 2]     = src[i];
                dst[i * 2 + 1] = src[i];
              }
            bailian_player_write(app->player, upsample_buf, got * 2);
          }
        }
      else if (player_active && c_mmi_audio_recv_all())
        {
          printf("[PLAYER] TTS audio finished\n");
          bailian_player_stop(app->player);
          player_active = false;
        }
      else
        {
          usleep(5000);
        }
    }

  if (player_active)
    {
      bailian_player_stop(app->player);
    }
  return NULL;
}

static void *bailian_ws_loop(void *arg)
{
  struct bailian_app_s *app = (struct bailian_app_s *)arg;
  struct bailian_ws_client *client = NULL;
  char url[256];
  char headers[256];
  uint32_t ws_send_count = 0;
  uint32_t ws_bin_bytes = 0;
  uint32_t ws_bin_frames = 0;

  /* Separate recv buffer on heap — TTS binary frames can be >> 8KB */
  const size_t recv_buf_size = 64 * 1024;
  uint8_t *recv_buf = malloc(recv_buf_size);
  if (recv_buf == NULL)
    {
      printf("[WS] Failed to allocate recv buffer\n");
      return NULL;
    }
  printf("[WS] Thread started\n");
  fflush(stdout);
  while (app->running)
    {
      if (client == NULL)
        {
          char *host = c_mmi_get_wss_host();
          char *port = c_mmi_get_wss_port();
          char *api = c_mmi_get_wss_api();
          char *header = c_mmi_get_wss_header();

          if (host == NULL || port == NULL || api == NULL)
            {
              usleep(100000);
              continue;
            }

          snprintf(url, sizeof(url), "wss://%s:%s%s", host, port, api);
          if (header != NULL && header[0] != '\0')
            {
              snprintf(headers, sizeof(headers), "%s\r\n", header);
            }
          else
            {
              headers[0] = '\0';
            }

          printf("[WS] Connecting to %s\n", url);
          if (bailian_ws_connect(url, headers, 5000, &client) != 0)
            {
              client = NULL;
              usleep(200000);
              continue;
            }
        }

      /* 1. Send SDK text commands (run-task, SendSpeech, HeartBeat etc) */

      uint8_t opcode = 0;
      uint8_t buffer[8192];
      uint32_t size = c_mmi_get_send_data(&opcode, buffer, sizeof(buffer));
      if (size > 0)
        {
          if (opcode == WEBSOCKET_OPCODE_BINARY)
            {
              ws_bin_bytes += size;
              ws_bin_frames++;
              if (ws_bin_frames % 100 == 0)
                {
                  printf("[WS] SDK binary #%u size=%u total=%u\n",
                         ws_bin_frames, (unsigned)size, ws_bin_bytes);
                }
            }
          else
            {
              printf("[WS] text #%u size=%u: %.*s\n",
                     ws_send_count, (unsigned)size,
                     (int)(size > 512 ? 512 : size), (char *)buffer);
            }
          ws_send_count++;
          bailian_ws_send(client, opcode, buffer, size);
        }

      /* 2. Stream audio binary from our own buffer.
       *    Trigger: app->speech_started (set by SDK event callback).
       *    Send up to 8 frames per loop iteration. */
      if (app->speech_started && client != NULL)
        {
          int burst;
          for (burst = 0; burst < 8; burst++)
            {
              pthread_mutex_lock(&app->audio_mutex);
              uint32_t rp = app->audio_read_pos;
              uint32_t wp = app->audio_write_pos;
              uint32_t avail = (wp > rp) ? (wp - rp) : 0;
              if (avail > 0)
                {
                  uint32_t chunk = avail;
                  if (chunk > sizeof(buffer)) chunk = sizeof(buffer);
                  memcpy(buffer, app->audio_buf + rp, chunk);
                  app->audio_read_pos = rp + chunk;
                  pthread_mutex_unlock(&app->audio_mutex);
                  ws_bin_bytes += chunk;
                  ws_bin_frames++;
                  if (ws_bin_frames <= 3 || ws_bin_frames % 10 == 0)
                    {
                      printf("[WS] binary #%u size=%u total=%u\n",
                             ws_bin_frames, (unsigned)chunk,
                             ws_bin_bytes);
                    }
                  bailian_ws_send(client, WEBSOCKET_OPCODE_BINARY,
                                  buffer, chunk);
                }
              else
                {
                  pthread_mutex_unlock(&app->audio_mutex);
                  if (app->audio_done && app->stop_requested)
                    {
                      printf("[WS] audio drained, calling speech_end\n");
                      c_mmi_speech_end();
                      bailian_recorder_close(app->recorder);
                      app->recording = false;
                      app->speech_started = false;
                      app->stop_requested = false;
                      app->audio_done = false;
                      ws_bin_frames = 0;
                      ws_bin_bytes = 0;
                    }
                  break;
                }
            }
        }

      /* 3. Receive from server */

      memset(recv_buf, 0, 256);
      int ret = bailian_ws_recv(client, &opcode, recv_buf, recv_buf_size, 50);
      if (ret > 0)
        {
          if (opcode == WEBSOCKET_OPCODE_TEXT && ret < 1024)
            {
              printf("[WS] recv text: %.*s\n", ret, (char *)recv_buf);
            }
          else if (opcode == WEBSOCKET_OPCODE_BINARY)
            {
              printf("[WS] recv binary size=%d\n", ret);
            }

          c_mmi_analyze_recv_data(opcode, recv_buf, (uint32_t)ret);
          if (opcode == WEBSOCKET_OPCODE_DISCONNECT)
            {
              bailian_ws_close(client);
              client = NULL;
            }
        }
      else if (ret < 0)
        {
          printf("[WS] recv error=%d, reconnecting\n", ret);
          bailian_ws_close(client);
          client = NULL;
        }
    }

  if (client != NULL)
    {
      bailian_ws_close(client);
    }

  free(recv_buf);
  return NULL;
}

static int bailian_event_cb(uint32_t event, void *param)
{
  (void)param;

  switch (event)
    {
      case C_MMI_EVENT_USER_CONFIG:
        {
          /* Reset dialog ID per official doc */
          c_mmi_reset_dialog_id();
          mmi_user_config_t config = C_MMI_CONFIG_DEFAULT();
          config.evt_cb = bailian_event_cb;
          config.work_mode = C_MMI_MODE_PUSH2TALK;
          config.text_mode = C_MMI_TEXT_MODE_BOTH;
          config.upstream_mode = C_MMI_STREAM_MODE_PCM;
          config.downstream_mode = C_MMI_STREAM_MODE_PCM;
          config.us_sample_rate = 24000;
          config.ds_sample_rate = 24000;
          config.recorder_rb_size = 512 * 1024;
          config.player_rb_size = 64 * 1024;
          config.user_id = "openvela_user";
          strncpy(config.voice_id, "longxiaochun_v3",
                  sizeof(config.voice_id) - 1);
          strncpy(config.story_voice_id, "longxiaochun_v3",
                  sizeof(config.story_voice_id) - 1);
          c_mmi_config(&config);
        }
        break;
      case C_MMI_EVENT_DATA_INIT:
        /* SDK initialized, WSS connection will be established in ws_loop */
        printf("[BAILIAN] SDK data init done, ready for WSS\n");
        break;
      case C_MMI_EVENT_SPEECH_START:
      case C_MMI_EVENT_SPEECH_RESTART:
        g_bailian.speech_started = true;
        printf("[BAILIAN] event: SpeechStart/Restart\n");
        break;
        break;
      case C_MMI_EVENT_SPEECH_PREPARE:
        g_bailian.speech_started = false;
        printf("[BAILIAN] event: SpeechPrepare\n");
        break;
      case C_MMI_EVENT_TTS_START:
        g_bailian.playing = true;
        break;
      case C_MMI_EVENT_TTS_END:
        g_bailian.playing = false;
        break;
      default:
        break;
    }

  return 0;
}

/* bailian_register_and_login() removed - not needed for pay-as-you-go mode */

static int bailian_init_app(void)
{
  g_bailian.recorder = bailian_recorder_create(CONFIG_AI_BAILIAN_RECORDER_DEV,
                                               24000, 1, 16);
  g_bailian.player = bailian_player_create(CONFIG_AI_BAILIAN_PLAYER_DEV,
                                           48000, 1, 16);
  if (g_bailian.recorder == NULL || g_bailian.player == NULL)
    {
      return -ENODEV;
    }
  /* Allocate our own audio buffer (512KB) */
  g_bailian.audio_buf_size = 512 * 1024;
  g_bailian.audio_buf = malloc(g_bailian.audio_buf_size);
  if (g_bailian.audio_buf == NULL)
    {
      return -ENOMEM;
    }
  g_bailian.audio_write_pos = 0;
  g_bailian.audio_read_pos = 0;
  g_bailian.audio_done = false;
  pthread_mutex_init(&g_bailian.audio_mutex, NULL);
  g_bailian.running = true;
  /* Use large stack for all threads - SDK internal processing needs it */
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 32768);
  pthread_create(&g_bailian.recorder_thread, &attr, bailian_recorder_loop,
                 &g_bailian);
  pthread_create(&g_bailian.player_thread, &attr, bailian_player_loop,
                 &g_bailian);
  pthread_create(&g_bailian.ws_thread, &attr, bailian_ws_loop, &g_bailian);
  pthread_attr_destroy(&attr);
  return 0;
}

static void bailian_deinit_app(void)
{
  g_bailian.running = false;
  pthread_join(g_bailian.recorder_thread, NULL);
  pthread_join(g_bailian.player_thread, NULL);
  pthread_join(g_bailian.ws_thread, NULL);
  bailian_recorder_destroy(g_bailian.recorder);
  bailian_player_destroy(g_bailian.player);
  g_bailian.recorder = NULL;
  g_bailian.player = NULL;
  if (g_bailian.audio_buf != NULL)
    {
      free(g_bailian.audio_buf);
      g_bailian.audio_buf = NULL;
    }
  pthread_mutex_destroy(&g_bailian.audio_mutex);
}

int main(int argc, FAR char *argv[])
{
  (void)argc;
  (void)argv;

  /* Ensure storage directory exists before any test */
  mkdir("/data/bailian", 0755);
  sync();  /* Ensure directory is persisted to filesystem */

  if (argc > 1 && strcmp(argv[1], "hal_test") == 0)
    {
      printf("=== Bailian Official HAL Test ===\n");
      /* Do NOT erase storage before test - SDK expects to read previous run's data */
      int32_t ret = qwen_sdk_test();
      printf("=== HAL Test Result: %d ===\n", ret);
      return ret;
    }

  if (argc > 1 && strcmp(argv[1], "test") == 0)
    {
      printf("=== Bailian HAL Test ===\n");

      printf("[MEM] util_malloc(100)... ");
      void *p = util_malloc(100);
      if (p != NULL)
        {
          printf("OK (ptr=%p)\n", p);
          util_free(p);
        }
      else
        {
          printf("FAIL\n");
        }

      printf("[TIME] util_now_ms()... ");
      int64_t t1 = util_now_ms();
      usleep(10000);
      int64_t t2 = util_now_ms();
      if (t2 > t1)
        {
          printf("OK (delta=%lld ms)\n", (long long)(t2 - t1));
        }
      else
        {
          printf("FAIL (t1=%lld, t2=%lld)\n", (long long)t1, (long long)t2);
        }

      printf("[RANDOM] util_random_init + util_random()... ");
      util_random_init(12345);
      uint32_t r1 = util_random();
      uint32_t r2 = util_random();
      if (r1 != r2)
        {
          printf("OK (r1=%u, r2=%u)\n", r1, r2);
        }
      else
        {
          printf("WARN (same values)\n");
        }

      printf("[MUTEX] create/lock/unlock/delete... ");
      util_mutex_t *mtx = util_mutex_create();
      if (mtx != NULL)
        {
          int ret = util_mutex_lock(mtx, 100);
          if (ret == UTIL_SUCCESS)
            {
              util_mutex_unlock(mtx);
              util_mutex_delete(mtx);
              printf("OK\n");
            }
          else
            {
              printf("FAIL (lock ret=%d)\n", ret);
            }
        }
      else
        {
          printf("FAIL (create)\n");
        }

      printf("[STORAGE] erase/storage/load... ");
      util_storage_erase();
      uint8_t wdata[16] = "BAILIAN_TEST";
      uint8_t rdata[16] = {0};
      int ret = util_storage_storage(wdata, sizeof(wdata));
      if (ret == UTIL_SUCCESS)
        {
          ret = util_storage_load(rdata, sizeof(rdata));
          if (ret == UTIL_SUCCESS && memcmp(wdata, rdata, sizeof(wdata)) == 0)
            {
              printf("OK\n");
            }
          else
            {
              printf("FAIL (load ret=%d)\n", ret);
            }
        }
      else
        {
          printf("FAIL (storage ret=%d)\n", ret);
        }

      printf("=== HAL Test Complete ===\n");
      return 0;
    }

  if (argc > 1 && strcmp(argv[1], "test_net") == 0)
    {
      printf("=== Bailian Network Test ===\n");

      printf("[HTTP] Testing HTTP POST to httpbin.org... ");
      char response[1024];
      int ret = bailian_http_post("http://httpbin.org/post",
                                  "application/json",
                                  "{\"test\":\"bailian\"}",
                                  response, sizeof(response), 10000);
      if (ret > 0)
        {
          printf("OK (%d bytes)\n", ret);
        }
      else
        {
          printf("FAIL (ret=%d)\n", ret);
        }

      printf("=== Network Test Complete ===\n");
      return 0;
    }

  if (argc > 1 && strcmp(argv[1], "test_audio") == 0)
    {
      printf("=== Bailian Audio Test ===\n");

      printf("[PLAYER] Testing WAV playback...\n");
      const char *test_wav = "/data/data/test.wav";
      struct bailian_player *player = bailian_player_create_from_file(test_wav);
      if (player != NULL)
        {
          printf("[PLAYER] File opened, starting playback...\n");
          if (bailian_player_start(player) == 0)
            {
              printf("[PLAYER] Playing... (waiting for completion)\n");
              while (!bailian_player_is_stopped(player))
                {
                  usleep(100000);
                }
              printf("[PLAYER] Playback finished\n");
            }
          else
            {
              printf("[PLAYER] Failed to start\n");
            }

          bailian_player_destroy(player);
        }
      else
        {
          printf("[PLAYER] Failed to open %s\n", test_wav);
        }

      printf("[RECORDER] Testing nxrecorder...\n");
      struct bailian_recorder *rec = bailian_recorder_create(
          CONFIG_AI_BAILIAN_RECORDER_DEV, 16000, 1, 16);
      if (rec != NULL)
        {
          printf("[RECORDER] Created OK\n");
          bailian_recorder_destroy(rec);
        }
      else
        {
          printf("[RECORDER] Create failed (nxrecorder not available)\n");
        }

      printf("=== Audio Test Complete ===\n");
      return 0;
    }

  if (argc > 1 && strcmp(argv[1], "help") == 0)
    {
      printf("Usage: bailian [command]\n");
      printf("Commands:\n");
      printf("  test        - Test HAL layer (mem, time, random, mutex, storage)\n");
      printf("  test_net    - Test network layer (HTTP)\n");
      printf("  test_audio  - Test audio layer (recorder, player)\n");
      printf("  help        - Show this help\n");
      printf("  (no args)   - Run full application\n");
      return 0;
    }

  /* 1. Initialize SDK first (per official doc) */
  if (c_mmi_sdk_init() != 0)
    {
      printf("bailian sdk init failed\n");
      return -1;
    }

  /* 2. Register event callback first (required by SDK) */
  c_mmi_register_event_callback(bailian_event_cb);

  /* 3. Initialize MMI for pay-as-you-go mode */
  if (c_mmi_init(CONFIG_AI_BAILIAN_WORKSPACE_ID,
                 CONFIG_AI_BAILIAN_APP_ID,
                 CONFIG_AI_BAILIAN_API_KEY) != 0)
    {
      printf("bailian mmi init failed\n");
      return -1;
    }

  /* Pre-resolve WSS host DNS in main thread.
   * getaddrinfo blocks in child threads on NuttX,
   * so we cache the result here for the WS thread.
   */

  bailian_ws_dns_prefetch("dashscope.aliyuncs.com", 443);

  if (bailian_init_app() != 0)
    {
      printf("bailian audio init failed\n");
      return -1;
    }

  printf("bailian ready: press 't' to talk, 's' to stop, 'q' to quit\n");
  for (;;)
    {
      int ch = getchar();
      if (ch == 't')
        {
          /* Reset our audio buffer for new session */
          pthread_mutex_lock(&g_bailian.audio_mutex);
          g_bailian.audio_write_pos = 0;
          g_bailian.audio_read_pos = 0;
          g_bailian.audio_done = false;
          pthread_mutex_unlock(&g_bailian.audio_mutex);
          /* Open streaming capture — use file if available, else device */
          int file_test = open("/data/speech_24k.pcm", O_RDONLY);
          if (file_test >= 0)
            {
              close(file_test);
              printf("[bailian] Using PCM file injection mode\n");
              bailian_recorder_open(g_bailian.recorder,
                                   "/data/speech_24k.pcm",
                                   24000, 1, 16);
            }
          else
            {
              bailian_recorder_open(g_bailian.recorder,
                                   CONFIG_AI_BAILIAN_RECORDER_DEV,
                                   24000, 1, 16);
            }
          g_bailian.recording = true;
          g_bailian.speech_started = false;
          g_bailian.stop_requested = false;
          g_bailian.record_start_ms = bailian_now_ms();
          c_mmi_speech_start();
          printf("[bailian] recording started at %lld ms\n",
                 (long long)g_bailian.record_start_ms);
        }

      else if (ch == 's')
        {
          /* Stop recording, let ws_loop drain ring buffer then speech_end */
          g_bailian.recording = false;
          g_bailian.stop_requested = true;
          printf("[bailian] stop requested, ws_loop will drain audio and call speech_end\n");
        }
      else if (ch == 'q')
        {
          break;
        }
      else
        {
          usleep(10000);
        }
    }

  bailian_deinit_app();
  c_mmi_deinit();
  return 0;
}
