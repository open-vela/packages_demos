/****************************************************************************
 * apps/ai/bailian/audio/player.c
 *
 * Audio player using nxaudio library for openvela bailian SDK
 ****************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <nuttx/config.h>

#ifdef CONFIG_AUDIOUTILS_NXAUDIO_LIB
#include <nuttx/audio/audio.h>
#include <audioutils/nxaudio.h>
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define PLAYER_STATE_IDLE     0
#define PLAYER_STATE_INIT     1
#define PLAYER_STATE_PLAYING  2
#define PLAYER_STATE_PAUSED   3
#define PLAYER_STATE_STOPPED  4

#define PLAYER_MODE_FILE      0
#define PLAYER_MODE_STREAM    1

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* WAV file header structures */

struct wav_riff_s
{
  char     chunk_id[4];    /* "RIFF" */
  uint32_t chunk_size;
  char     format[4];      /* "WAVE" */
};

struct wav_fmt_s
{
  char     subchunk1_id[4];   /* "fmt " */
  uint32_t subchunk1_size;
  uint16_t audio_format;
  uint16_t num_channels;
  uint32_t sample_rate;
  uint32_t byte_rate;
  uint16_t block_align;
  uint16_t bits_per_sample;
};

struct wav_data_s
{
  char     subchunk2_id[4];   /* "data" */
  uint32_t subchunk2_size;
};

struct wav_header_s
{
  struct wav_riff_s riff;
  struct wav_fmt_s  fmt;
  struct wav_data_s data;
};

struct bailian_player
{
#ifdef CONFIG_AUDIOUTILS_NXAUDIO_LIB
  struct nxaudio_s nxaudio;
#endif
  struct wav_header_s wav;
  int fd;                    /* Read fd (file or pipe read end) */
  int write_fd;              /* Pipe write end for streaming mode */
  volatile int state;
  pthread_t thread_id;
  uint32_t file_position;
  int mode;                  /* PLAYER_MODE_FILE or PLAYER_MODE_STREAM */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

#ifdef CONFIG_AUDIOUTILS_NXAUDIO_LIB
static void player_dequeue_cb(unsigned long arg, FAR struct ap_buffer_s *apb);
static void player_complete_cb(unsigned long arg);
static void player_user_cb(unsigned long arg, FAR struct audio_msg_s *msg,
                           FAR bool *running);
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

#ifdef CONFIG_AUDIOUTILS_NXAUDIO_LIB
static struct nxaudio_callbacks_s g_player_cbs =
{
  player_dequeue_cb,
  player_complete_cb,
  player_user_cb
};
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

#ifdef CONFIG_AUDIOUTILS_NXAUDIO_LIB
static void player_dequeue_cb(unsigned long arg, FAR struct ap_buffer_s *apb)
{
  FAR struct bailian_player *player = (FAR struct bailian_player *)(uintptr_t)arg;
  ssize_t bytes_read;

  if (!apb || !player)
    {
      return;
    }

  if (player->fd < 0)
    {
      apb->nbytes = 0;
      return;
    }

  bytes_read = read(player->fd, apb->samp, apb->nmaxbytes);
  if (bytes_read < 0)
    {
      apb->nbytes = 0;
      return;
    }

  apb->nbytes = (size_t)bytes_read;
  apb->curbyte = 0;
  apb->flags = 0;

  while (apb->nbytes < apb->nmaxbytes)
    {
      int n = apb->nmaxbytes - apb->nbytes;
      int ret = read(player->fd, &apb->samp[apb->nbytes], n);
      if (ret <= 0)
        {
          break;
        }

      apb->nbytes += ret;
    }

  player->file_position += apb->nbytes;

  if (apb->nbytes < apb->nmaxbytes)
    {
      if (player->mode == PLAYER_MODE_FILE)
        {
          close(player->fd);
          player->fd = -1;
        }

      if (apb->nbytes > 0)
        {
          apb->flags = AUDIO_APB_FINAL;
          nxaudio_enqbuffer(&player->nxaudio, apb);
        }

      return;
    }

  nxaudio_enqbuffer(&player->nxaudio, apb);
}

static void player_complete_cb(unsigned long arg)
{
  FAR struct bailian_player *player = (FAR struct bailian_player *)(uintptr_t)arg;

  if (player)
    {
      player->state = PLAYER_STATE_STOPPED;
    }
}

static void player_user_cb(unsigned long arg, FAR struct audio_msg_s *msg,
                           FAR bool *running)
{
  /* Not used */
}

static FAR void *player_thread(FAR void *arg)
{
  FAR struct bailian_player *player = (FAR struct bailian_player *)arg;
  int i;

  /* Pre-fill audio buffers before starting playback.
   * Without this, the driver starts with no enqueued buffers,
   * never triggers dequeue callbacks, and no audio plays.
   * For stream mode the pipe read-end is blocking, so this
   * will wait until bailian_player_write() pushes data. */

  for (i = 0; i < player->nxaudio.abufnum; i++)
    {
      player_dequeue_cb((unsigned long)(uintptr_t)player,
                        player->nxaudio.abufs[i]);
    }

  nxaudio_start(&player->nxaudio);
  nxaudio_msgloop(&player->nxaudio, &g_player_cbs,
                  (unsigned long)(uintptr_t)player);

  player->state = PLAYER_STATE_STOPPED;
  return NULL;
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

struct bailian_player *bailian_player_create_from_file(const char *filepath)
{
#ifdef CONFIG_AUDIOUTILS_NXAUDIO_LIB
  struct bailian_player *player;
  ssize_t read_ret;
  int ret;
  int i;

  player = calloc(1, sizeof(struct bailian_player));
  if (player == NULL)
    {
      return NULL;
    }

  player->state = PLAYER_STATE_IDLE;
  player->mode = PLAYER_MODE_FILE;
  player->fd = -1;
  player->write_fd = -1;

  player->fd = open(filepath, O_RDONLY);
  if (player->fd < 0)
    {
      free(player);
      return NULL;
    }

  read_ret = read(player->fd, &player->wav, sizeof(player->wav));
  if (read_ret < (ssize_t)sizeof(player->wav))
    {
      close(player->fd);
      free(player);
      return NULL;
    }

  if (memcmp(player->wav.riff.chunk_id, "RIFF", 4) != 0 ||
      memcmp(player->wav.riff.format, "WAVE", 4) != 0)
    {
      close(player->fd);
      free(player);
      return NULL;
    }

  ret = init_nxaudio(&player->nxaudio,
                     (int)player->wav.fmt.sample_rate,
                     (int)player->wav.fmt.bits_per_sample,
                     (int)player->wav.fmt.num_channels);
  if (ret < 0)
    {
      close(player->fd);
      free(player);
      return NULL;
    }

  for (i = 0; i < player->nxaudio.abufnum; i++)
    {
      player_dequeue_cb((unsigned long)(uintptr_t)player,
                        player->nxaudio.abufs[i]);
    }

  player->state = PLAYER_STATE_INIT;

  return player;
#else
  return NULL;
#endif
}

int bailian_player_start(struct bailian_player *player)
{
#ifdef CONFIG_AUDIOUTILS_NXAUDIO_LIB
  pthread_attr_t attr;
  struct sched_param param;
  int ret;

  if (player == NULL)
    {
      return -EINVAL;
    }

  if (player->state != PLAYER_STATE_INIT &&
      player->state != PLAYER_STATE_PAUSED)
    {
      return -EPERM;
    }

  player->state = PLAYER_STATE_PLAYING;

  pthread_attr_init(&attr);
  param.sched_priority = sched_get_priority_max(SCHED_FIFO) - 9;
  pthread_attr_setschedparam(&attr, &param);
  pthread_attr_setstacksize(&attr, 4096);

  ret = pthread_create(&player->thread_id, &attr, player_thread, player);
  pthread_attr_destroy(&attr);

  if (ret != 0)
    {
      player->state = PLAYER_STATE_INIT;
      return -ret;
    }

  pthread_setname_np(player->thread_id, "bailian_player");

  return 0;
#else
  return -ENOSYS;
#endif
}

int bailian_player_stop(struct bailian_player *player)
{
#ifdef CONFIG_AUDIOUTILS_NXAUDIO_LIB
  if (player == NULL)
    {
      return -EINVAL;
    }

  /* Always call nxaudio_stop if thread is running, regardless of state.
   * This ensures msgloop receives AUDIO_MSG_STOP and exits.
   * Even if playback completed naturally (state=STOPPED), the thread
   * may still be blocked on mq_receive().
   */

  if (player->thread_id > 0)
    {
      player->state = PLAYER_STATE_STOPPED;

      if (player->write_fd >= 0)
        {
          close(player->write_fd);
          player->write_fd = -1;
        }

      nxaudio_stop(&player->nxaudio);
      pthread_join(player->thread_id, NULL);
      player->thread_id = 0;
    }

  return 0;
#else
  return -ENOSYS;
#endif
}

int bailian_player_set_volume(struct bailian_player *player, uint16_t volume)
{
#ifdef CONFIG_AUDIOUTILS_NXAUDIO_LIB
  if (player == NULL)
    {
      return -EINVAL;
    }

  return nxaudio_setvolume(&player->nxaudio, volume);
#else
  return -ENOSYS;
#endif
}

void bailian_player_destroy(struct bailian_player *player)
{
#ifdef CONFIG_AUDIOUTILS_NXAUDIO_LIB
  if (player == NULL)
    {
      return;
    }

  bailian_player_stop(player);

  if (player->fd >= 0)
    {
      close(player->fd);
      player->fd = -1;
    }

  if (player->write_fd >= 0)
    {
      close(player->write_fd);
      player->write_fd = -1;
    }

  fin_nxaudio(&player->nxaudio);
  free(player);
#endif
}

/* Legacy API - kept for compatibility */

struct bailian_player *bailian_player_create(const char *dev, int sample_rate,
                                             int channels, int bits_per_sample)
{
#ifdef CONFIG_AUDIOUTILS_NXAUDIO_LIB
  struct bailian_player *player;
  int pipefd[2];
  int ret;

  (void)dev;

  player = calloc(1, sizeof(struct bailian_player));
  if (player == NULL)
    {
      return NULL;
    }

  player->state = PLAYER_STATE_IDLE;
  player->mode = PLAYER_MODE_STREAM;
  player->fd = -1;
  player->write_fd = -1;

  ret = pipe(pipefd);
  if (ret < 0)
    {
      free(player);
      return NULL;
    }

  player->fd = pipefd[0];
  player->write_fd = pipefd[1];

  /* Set write end non-blocking so player_write won't block when
   * nxaudio/virtio-snd can't consume fast enough.
   * This prevents player_rb from filling up and stalling SDK recv. */
  {
    int flags = fcntl(player->write_fd, F_GETFL, 0);
    if (flags >= 0)
      {
        fcntl(player->write_fd, F_SETFL, flags | O_NONBLOCK);
      }
  }
  ret = init_nxaudio(&player->nxaudio, sample_rate, bits_per_sample, channels);
  if (ret < 0)
    {
      close(player->fd);
      close(player->write_fd);
      free(player);
      return NULL;
    }

  player->state = PLAYER_STATE_INIT;

  return player;
#else
  return NULL;
#endif
}

int bailian_player_open(struct bailian_player *player, const char *dev,
                        int sample_rate, int channels, int bits_per_sample)
{
  return -ENOSYS;
}

ssize_t bailian_player_write(struct bailian_player *player,
                             const uint8_t *buffer, size_t len)
{
#ifdef CONFIG_AUDIOUTILS_NXAUDIO_LIB
  ssize_t written = 0;
  ssize_t ret;

  if (player == NULL || buffer == NULL)
    {
      return -EINVAL;
    }

  if (player->mode != PLAYER_MODE_STREAM)
    {
      return -EPERM;
    }

  if (player->write_fd < 0)
    {
      return -EBADF;
    }

  while (written < (ssize_t)len)
    {
      ret = write(player->write_fd, buffer + written, len - written);
      if (ret < 0)
        {
          if (errno == EINTR)
            {
              continue;
            }

          if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
              /* Pipe full — nxaudio can't consume fast enough.
               * Drop remaining data to keep player_rb draining. */
              break;
            }

          return written > 0 ? written : -errno;
        }

      if (ret == 0)
        {
          break;
        }

      written += ret;
    }

  return written;
#else
  return -ENOSYS;
#endif
}

int bailian_player_is_stopped(struct bailian_player *player)
{
  if (player == NULL)
    {
      return 1;
    }

  return player->state == PLAYER_STATE_STOPPED;
}

void bailian_player_close(struct bailian_player *player)
{
  bailian_player_destroy(player);
}
