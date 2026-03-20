#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <nuttx/config.h>
#include <nuttx/audio/audio.h>

#ifdef CONFIG_SYSTEM_NXRECORDER
#include <system/nxrecorder.h>
#endif

#define RECORDER_STATE_IDLE      0
#define RECORDER_STATE_RECORDING 1
#define RECORDER_STATE_PAUSED    2
#define RECORDER_STATE_STOPPED   3

#define RECORDER_NUM_BUFFERS     4
#define RECORDER_BUFFER_SIZE     4096
#define RECORDER_MQ_MAX_MSGS     8
#define RECORDER_MQ_NAME_FMT     "/tmp/rec%lx"

struct bailian_recorder
{
#ifdef CONFIG_SYSTEM_NXRECORDER
  FAR struct nxrecorder_s *nxrec;
#endif
  int state;
  int sample_rate;
  int channels;
  int bits_per_sample;
  char filepath[128];

  /* Streaming capture fields */

  int dev_fd;
  mqd_t mq;
  char mqname[32];
  FAR struct ap_buffer_s **buffers;
  int nbuffers;

  /* Partial buffer tracking for read() */

  FAR struct ap_buffer_s *cur_apb;
  uint32_t cur_offset;

  /* File injection mode (bypass audio device) */

  int file_mode;
  int file_fd;
};

/* Forward declarations */

static int recorder_stream_stop(struct bailian_recorder *rec);

/****************************************************************************
 * nxrecorder-based API (file recording)
 ****************************************************************************/

struct bailian_recorder *bailian_recorder_create(const char *dev,
                                                 int sample_rate,
                                                 int channels,
                                                 int bits_per_sample)
{
#ifdef CONFIG_SYSTEM_NXRECORDER
  struct bailian_recorder *rec;

  rec = calloc(1, sizeof(*rec));
  if (rec == NULL)
    {
      return NULL;
    }

  rec->nxrec = nxrecorder_create();
  if (rec->nxrec == NULL)
    {
      printf("[bailian] Failed to create nxrecorder\n");
      free(rec);
      return NULL;
    }

  if (dev != NULL)
    {
      nxrecorder_setdevice(rec->nxrec, dev);
    }

  rec->sample_rate = sample_rate;
  rec->channels = channels;
  rec->bits_per_sample = bits_per_sample;
  rec->state = RECORDER_STATE_IDLE;
  rec->dev_fd = -1;
  rec->mq = (mqd_t)-1;
  rec->file_fd = -1;
  rec->file_mode = 0;
  rec->buffers = NULL;
  rec->cur_apb = NULL;
  rec->cur_offset = 0;

  return rec;
#else
  printf("[bailian] SYSTEM_NXRECORDER not enabled\n");
  return NULL;
#endif
}

int bailian_recorder_start(struct bailian_recorder *rec, const char *filepath)
{
#ifdef CONFIG_SYSTEM_NXRECORDER
  int ret;

  if (rec == NULL || rec->nxrec == NULL || filepath == NULL)
    {
      return -EINVAL;
    }

  strncpy(rec->filepath, filepath, sizeof(rec->filepath) - 1);

  ret = nxrecorder_recordinternal(rec->nxrec, filepath,
                                  AUDIO_FMT_PCM, rec->channels,
                                  rec->bits_per_sample, rec->sample_rate, 0);
  if (ret < 0)
    {
      printf("[bailian] Failed to start recording: %d\n", ret);
      return ret;
    }

  rec->state = RECORDER_STATE_RECORDING;
  printf("[bailian] Recording started: %s\n", filepath);

  return 0;
#else
  return -ENOSYS;
#endif
}

int bailian_recorder_stop(struct bailian_recorder *rec)
{
#ifdef CONFIG_SYSTEM_NXRECORDER
  if (rec == NULL || rec->nxrec == NULL)
    {
      return -EINVAL;
    }

  if (rec->state != RECORDER_STATE_RECORDING &&
      rec->state != RECORDER_STATE_PAUSED)
    {
      return 0;
    }

#ifndef CONFIG_AUDIO_EXCLUDE_STOP
  nxrecorder_stop(rec->nxrec);
#endif

  rec->state = RECORDER_STATE_STOPPED;
  printf("[bailian] Recording stopped\n");

  return 0;
#else
  return -ENOSYS;
#endif
}

int bailian_recorder_pause(struct bailian_recorder *rec)
{
#ifdef CONFIG_SYSTEM_NXRECORDER
  if (rec == NULL || rec->nxrec == NULL)
    {
      return -EINVAL;
    }

  if (rec->state != RECORDER_STATE_RECORDING)
    {
      return -EPERM;
    }

#ifndef CONFIG_AUDIO_EXCLUDE_PAUSE_RESUME
  nxrecorder_pause(rec->nxrec);
#endif

  rec->state = RECORDER_STATE_PAUSED;
  return 0;
#else
  return -ENOSYS;
#endif
}

int bailian_recorder_resume(struct bailian_recorder *rec)
{
#ifdef CONFIG_SYSTEM_NXRECORDER
  if (rec == NULL || rec->nxrec == NULL)
    {
      return -EINVAL;
    }

  if (rec->state != RECORDER_STATE_PAUSED)
    {
      return -EPERM;
    }

#ifndef CONFIG_AUDIO_EXCLUDE_PAUSE_RESUME
  nxrecorder_resume(rec->nxrec);
#endif

  rec->state = RECORDER_STATE_RECORDING;
  return 0;
#else
  return -ENOSYS;
#endif
}

void bailian_recorder_destroy(struct bailian_recorder *rec)
{
#ifdef CONFIG_SYSTEM_NXRECORDER
  if (rec == NULL)
    {
      return;
    }

  /* Stop streaming capture if active */

  if (rec->dev_fd >= 0)
    {
      recorder_stream_stop(rec);
    }

  bailian_recorder_stop(rec);

  if (rec->nxrec != NULL)
    {
      nxrecorder_release(rec->nxrec);
      rec->nxrec = NULL;
    }

  free(rec);
#endif
}

/****************************************************************************
 * Streaming capture API (open/read/close for SDK real-time audio)
 ****************************************************************************/

int bailian_recorder_open(struct bailian_recorder *rec, const char *dev,
                          int sample_rate, int channels, int bits_per_sample)
{
  struct audio_caps_desc_s cap_desc;
  struct audio_buf_desc_s buf_desc;
  struct ap_buffer_info_s buf_info;
  struct mq_attr attr;
  int ret;
  int i;

  if (rec == NULL)
    {
      return -EINVAL;
    }

  if (rec->dev_fd >= 0 || rec->file_fd >= 0)
    {
      return -EBUSY;
    }

  /* Use provided dev or fall back to stored device path */

  if (dev == NULL)
    {
      dev = CONFIG_AI_BAILIAN_RECORDER_DEV;
    }

  /* Check if dev is a PCM file (file injection mode) */

  size_t devlen = strlen(dev);
  if (devlen > 4 && strcmp(dev + devlen - 4, ".pcm") == 0)
    {
      rec->file_fd = open(dev, O_RDONLY);
      if (rec->file_fd < 0)
        {
          printf("[bailian] recorder: failed to open file %s: %d\n",
                 dev, errno);
          return -errno;
        }

      rec->file_mode = 1;
      rec->sample_rate = sample_rate ? sample_rate : 24000;
      rec->channels = channels ? channels : 1;
      rec->bits_per_sample = bits_per_sample ? bits_per_sample : 16;
      rec->state = RECORDER_STATE_RECORDING;
      printf("[bailian] recorder: FILE MODE from %s (%dHz %dch %dbit)\n",
             dev, rec->sample_rate, rec->channels, rec->bits_per_sample);
      return OK;
    }

  /* Warmup: QEMU virtio-snd PA capture needs a dummy open/start cycle
   * to activate the PulseAudio source (first open returns zeros). */

  {
    int warmup_fd = open(dev, O_RDWR | O_CLOEXEC);
    if (warmup_fd >= 0)
      {
        struct audio_caps_desc_s wcap;
        memset(&wcap, 0, sizeof(wcap));
        wcap.caps.ac_len = sizeof(struct audio_caps_s);
        wcap.caps.ac_type = AUDIO_TYPE_INPUT;
        wcap.caps.ac_channels = channels ? channels : 1;
        wcap.caps.ac_controls.hw[0] = sample_rate ? sample_rate : 24000;
        wcap.caps.ac_controls.b[3] = sample_rate >> 16;
        wcap.caps.ac_controls.b[2] = bits_per_sample ? bits_per_sample : 16;
        wcap.caps.ac_subtype = AUDIO_FMT_PCM;

        if (ioctl(warmup_fd, AUDIOIOC_RESERVE, 0) == 0)
          {
            ioctl(warmup_fd, AUDIOIOC_CONFIGURE, (uintptr_t)&wcap);
            ioctl(warmup_fd, AUDIOIOC_START, 0);
            usleep(200000); /* 200ms - let PA source activate */
            ioctl(warmup_fd, AUDIOIOC_STOP, 0);
            ioctl(warmup_fd, AUDIOIOC_RELEASE, 0);
          }

        close(warmup_fd);
        usleep(100000); /* 100ms - let device settle */
        printf("[bailian] recorder: warmup cycle done\n");
      }
  }

  /* Open the audio capture device */

  rec->dev_fd = open(dev, O_RDWR | O_CLOEXEC);
  if (rec->dev_fd < 0)
    {
      printf("[bailian] recorder: failed to open %s: %d\n", dev, errno);
      return -errno;
    }

  /* Reserve the device */

  ret = ioctl(rec->dev_fd, AUDIOIOC_RESERVE, 0);
  if (ret < 0)
    {
      printf("[bailian] recorder: RESERVE failed: %d\n", errno);
      goto err_close;
    }

  /* Configure for capture */

  memset(&cap_desc, 0, sizeof(cap_desc));
  cap_desc.caps.ac_len = sizeof(struct audio_caps_s);
  cap_desc.caps.ac_type = AUDIO_TYPE_INPUT;
  cap_desc.caps.ac_channels = channels ? channels : 1;
  cap_desc.caps.ac_controls.hw[0] = sample_rate ? sample_rate : 16000;
  cap_desc.caps.ac_controls.b[3] = sample_rate >> 16;
  cap_desc.caps.ac_controls.b[2] = bits_per_sample ? bits_per_sample : 16;
  cap_desc.caps.ac_subtype = AUDIO_FMT_PCM;

  ret = ioctl(rec->dev_fd, AUDIOIOC_CONFIGURE, (uintptr_t)&cap_desc);
  if (ret < 0)
    {
      printf("[bailian] recorder: CONFIGURE failed: %d\n", errno);
      goto err_release;
    }

  /* Query buffer info */

  if (ioctl(rec->dev_fd, AUDIOIOC_GETBUFFERINFO,
            (uintptr_t)&buf_info) != OK)
    {
      buf_info.buffer_size = RECORDER_BUFFER_SIZE;
      buf_info.nbuffers = RECORDER_NUM_BUFFERS;
    }

  rec->nbuffers = buf_info.nbuffers;

  /* Allocate buffer pointer array */

  rec->buffers = calloc(rec->nbuffers, sizeof(FAR struct ap_buffer_s *));
  if (rec->buffers == NULL)
    {
      printf("[bailian] recorder: buffer array alloc failed\n");
      ret = -ENOMEM;
      goto err_release;
    }

  /* Allocate audio buffers */

  for (i = 0; i < rec->nbuffers; i++)
    {
      buf_desc.numbytes = buf_info.buffer_size;
      buf_desc.u.pbuffer = &rec->buffers[i];

      ret = ioctl(rec->dev_fd, AUDIOIOC_ALLOCBUFFER,
                  (uintptr_t)&buf_desc);
      if (ret != sizeof(buf_desc))
        {
          printf("[bailian] recorder: ALLOCBUFFER %d failed\n", i);
          ret = -ENOMEM;
          goto err_free_bufs;
        }
    }

  /* Create message queue */

  snprintf(rec->mqname, sizeof(rec->mqname), RECORDER_MQ_NAME_FMT,
           (unsigned long)((uintptr_t)rec));

  attr.mq_maxmsg = rec->nbuffers + RECORDER_MQ_MAX_MSGS;
  attr.mq_msgsize = sizeof(struct audio_msg_s);
  attr.mq_curmsgs = 0;
  attr.mq_flags = 0;

  rec->mq = mq_open(rec->mqname, O_RDWR | O_CREAT, 0644, &attr);
  if (rec->mq == (mqd_t)-1)
    {
      printf("[bailian] recorder: mq_open failed: %d\n", errno);
      ret = -errno;
      goto err_free_bufs;
    }

  /* Register message queue with audio device */

  ioctl(rec->dev_fd, AUDIOIOC_REGISTERMQ, (uintptr_t)rec->mq);

  /* Enqueue all buffers for capture */

  for (i = 0; i < rec->nbuffers; i++)
    {
      struct audio_buf_desc_s enq;

      rec->buffers[i]->nbytes = rec->buffers[i]->nmaxbytes;
      enq.numbytes = rec->buffers[i]->nbytes;
      enq.u.buffer = rec->buffers[i];

      ret = ioctl(rec->dev_fd, AUDIOIOC_ENQUEUEBUFFER,
                  (uintptr_t)&enq);
      if (ret < 0)
        {
          printf("[bailian] recorder: ENQUEUEBUFFER %d failed: %d\n",
                 i, errno);
          goto err_mq;
        }
    }

  /* Start capture */

  ret = ioctl(rec->dev_fd, AUDIOIOC_START, 0);
  if (ret < 0)
    {
      printf("[bailian] recorder: START failed: %d\n", errno);
      goto err_mq;
    }

  rec->state = RECORDER_STATE_RECORDING;
  rec->cur_apb = NULL;
  rec->cur_offset = 0;

  printf("[bailian] recorder: streaming started (%dHz %dch %dbit)\n",
         sample_rate, channels, bits_per_sample);
  return 0;

err_mq:
  ioctl(rec->dev_fd, AUDIOIOC_UNREGISTERMQ, (uintptr_t)rec->mq);
  mq_close(rec->mq);
  mq_unlink(rec->mqname);
  rec->mq = (mqd_t)-1;

err_free_bufs:
  if (rec->buffers != NULL)
    {
      for (i = 0; i < rec->nbuffers; i++)
        {
          if (rec->buffers[i] != NULL)
            {
              buf_desc.u.buffer = rec->buffers[i];
              ioctl(rec->dev_fd, AUDIOIOC_FREEBUFFER,
                    (uintptr_t)&buf_desc);
            }
        }

      free(rec->buffers);
      rec->buffers = NULL;
    }

err_release:
  ioctl(rec->dev_fd, AUDIOIOC_RELEASE, 0);

err_close:
  close(rec->dev_fd);
  rec->dev_fd = -1;
  return ret;
}

ssize_t bailian_recorder_read(struct bailian_recorder *rec, uint8_t *buffer,
                              size_t len)
{
  struct audio_msg_s msg;
  unsigned int prio;
  size_t copied = 0;
  if (rec == NULL || buffer == NULL || len == 0)
    {
      return -EINVAL;
    }
  if (rec->state != RECORDER_STATE_RECORDING)
    {
      return -ENOSYS;
    }
  /* File injection mode: read from PCM file with pacing */
  if (rec->file_mode)
    {
      if (rec->file_fd < 0)
        {
          return -ENOSYS;
        }
      ssize_t n = read(rec->file_fd, buffer, len);
      if (n <= 0)
        {
          /* EOF or error - loop back to start */
          lseek(rec->file_fd, 0, SEEK_SET);
          n = read(rec->file_fd, buffer, len);
          if (n <= 0)
            {
              return 0;
            }
        }
      /* Pace to real-time: len bytes at sample_rate * channels * (bps/8) */
      int bytes_per_sec = rec->sample_rate * rec->channels *
                          (rec->bits_per_sample / 8);
      if (bytes_per_sec > 0)
        {
          useconds_t us = (useconds_t)((uint64_t)n * 1000000 / bytes_per_sec);
          usleep(us);
        }
      return n;
    }
  /* Audio device mode */
  if (rec->dev_fd < 0)
    {
      return -ENOSYS;
    }

  while (copied < len)
    {
      /* If we have leftover data from a previous buffer, use it */

      if (rec->cur_apb != NULL)
        {
          uint32_t avail = rec->cur_apb->nbytes - rec->cur_offset;
          uint32_t tocopy = (len - copied) < avail ?
                            (len - copied) : avail;

          memcpy(buffer + copied, rec->cur_apb->samp + rec->cur_offset,
                 tocopy);
          copied += tocopy;
          rec->cur_offset += tocopy;

          /* If buffer fully consumed, re-enqueue it */

          if (rec->cur_offset >= rec->cur_apb->nbytes)
            {
              struct audio_buf_desc_s enq;

              rec->cur_apb->nbytes = rec->cur_apb->nmaxbytes;
              enq.numbytes = rec->cur_apb->nbytes;
              enq.u.buffer = rec->cur_apb;

              ioctl(rec->dev_fd, AUDIOIOC_ENQUEUEBUFFER,
                    (uintptr_t)&enq);

              rec->cur_apb = NULL;
              rec->cur_offset = 0;
            }

          continue;
        }

      /* Wait for next filled buffer from audio driver */

      ssize_t sz = mq_receive(rec->mq, (FAR char *)&msg,
                              sizeof(msg), &prio);
      if (sz != sizeof(msg))
        {
          if (copied > 0)
            {
              return copied;
            }

          return -errno;
        }

      switch (msg.msg_id)
        {
          case AUDIO_MSG_DEQUEUE:
            rec->cur_apb = (FAR struct ap_buffer_s *)msg.u.ptr;
            rec->cur_offset = 0;
            break;

          case AUDIO_MSG_COMPLETE:
            rec->state = RECORDER_STATE_STOPPED;
            return copied > 0 ? copied : 0;

          default:
            break;
        }
    }

  return copied;
}

static int recorder_stream_stop(struct bailian_recorder *rec)
{
  struct audio_buf_desc_s buf_desc;
  struct audio_msg_s msg;
  unsigned int prio;
  struct timespec ts;
  int i;

  if (rec->dev_fd < 0)
    {
      return 0;
    }

  /* Stop the audio device */

  ioctl(rec->dev_fd, AUDIOIOC_STOP, 0);

  /* Drain remaining messages */

  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_nsec += 100000000; /* 100ms timeout */
  if (ts.tv_nsec >= 1000000000)
    {
      ts.tv_sec++;
      ts.tv_nsec -= 1000000000;
    }

  while (rec->mq != (mqd_t)-1)
    {
      if (mq_timedreceive(rec->mq, (FAR char *)&msg,
                          sizeof(msg), &prio, &ts) != sizeof(msg))
        {
          break;
        }
    }

  /* Free audio buffers */

  if (rec->buffers != NULL)
    {
      for (i = 0; i < rec->nbuffers; i++)
        {
          if (rec->buffers[i] != NULL)
            {
              buf_desc.u.buffer = rec->buffers[i];
              ioctl(rec->dev_fd, AUDIOIOC_FREEBUFFER,
                    (uintptr_t)&buf_desc);
            }
        }

      free(rec->buffers);
      rec->buffers = NULL;
    }

  /* Cleanup message queue */

  if (rec->mq != (mqd_t)-1)
    {
      ioctl(rec->dev_fd, AUDIOIOC_UNREGISTERMQ, (uintptr_t)rec->mq);
      mq_close(rec->mq);
      mq_unlink(rec->mqname);
      rec->mq = (mqd_t)-1;
    }

  /* Release and close device */

  ioctl(rec->dev_fd, AUDIOIOC_RELEASE, 0);
  close(rec->dev_fd);
  rec->dev_fd = -1;

  rec->cur_apb = NULL;
  rec->cur_offset = 0;
  rec->state = RECORDER_STATE_IDLE;

  printf("[bailian] recorder: streaming stopped\n");
  return 0;
}

void bailian_recorder_close(struct bailian_recorder *rec)
{
  if (rec == NULL)
    {
      return;
    }
  /* File injection mode cleanup */
  if (rec->file_mode)
    {
      if (rec->file_fd >= 0)
        {
          close(rec->file_fd);
          rec->file_fd = -1;
        }
      rec->file_mode = 0;
      rec->state = RECORDER_STATE_IDLE;
      printf("[bailian] recorder: file mode stopped\n");
      return;
    }
  recorder_stream_stop(rec);
}
