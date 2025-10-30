#ifndef AUDIO_CTL_H
#define AUDIO_CTL_H

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_FORMAT_UNKNOWN 0
#define AUDIO_FORMAT_WAV 1
#define AUDIO_FORMAT_MP3 2

#define AUDIO_CTL_STATE_STOP 0
#define AUDIO_CTL_STATE_START 1
#define AUDIO_CTL_STATE_PAUSE 2

typedef struct {
  uint32_t sample_rate;
  uint16_t num_channels;
  uint16_t bits_per_sample;
  uint32_t data_size;
  uint32_t data_offset;
} wav_s;

struct nxplayer_s;

typedef struct audioctl {
  char file_path[512];
  int audio_format;
  off_t file_size;

  struct nxplayer_s *nxplayer;

  volatile int state;
  volatile int is_playing;
  volatile int is_paused;
  volatile int should_stop;
  pthread_mutex_t control_mutex;

  uint32_t current_position_ms;
  uint32_t total_duration_ms;

  pthread_t monitor_thread;
  int monitor_running;

  wav_s wav;
  int fd;

  int seek;
  uint32_t seek_position;
  uint32_t file_position;
  pthread_t pid;
} audioctl_s;

int audio_ctl_detect_format(const char *path);
audioctl_s *audio_ctl_init_nxaudio(const char *path);
int audio_ctl_start(audioctl_s *ctl);
int audio_ctl_pause(audioctl_s *ctl);
int audio_ctl_resume(audioctl_s *ctl);
int audio_ctl_stop(audioctl_s *ctl);
int audio_ctl_set_volume(audioctl_s *ctl, uint16_t vol);
int audio_ctl_get_position(audioctl_s *ctl);
int audio_ctl_seek(audioctl_s *ctl, unsigned ms);
int audio_ctl_uninit_nxaudio(audioctl_s *ctl);

#ifdef __cplusplus
}
#endif

#endif
