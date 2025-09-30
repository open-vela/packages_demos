/**
 * Audio Controller Header - NXPlayer based
 * Vela system native audio player with MP3 support
 */

#ifndef AUDIO_CTL_H
#define AUDIO_CTL_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      DEFINES
 *********************/

/* Audio format definitions */
#define AUDIO_FORMAT_UNKNOWN 0
#define AUDIO_FORMAT_WAV     1
#define AUDIO_FORMAT_MP3     2

/* Playback state definitions */
#define AUDIO_CTL_STATE_STOP  0
#define AUDIO_CTL_STATE_START 1
#define AUDIO_CTL_STATE_PAUSE 2

/*********************
 *      TYPEDEFS
 *********************/

/* WAV file information structure */
typedef struct {
    uint32_t sample_rate;
    uint16_t num_channels;
    uint16_t bits_per_sample;
    uint32_t data_size;
    uint32_t data_offset;
} wav_s;

/* Forward declaration of NxPlayer structure */
struct nxplayer_s;

/* Audio controller structure - NxPlayer based */
typedef struct audioctl {
    // File information
    char file_path[512];
    int audio_format;
    off_t file_size;
    
    // NxPlayer instance - Vela system audio player
    struct nxplayer_s *nxplayer;
    
    // Playback state control
    volatile int state;
    volatile int is_playing;
    volatile int is_paused;
    volatile int should_stop;
    pthread_mutex_t control_mutex;
    
    // Playback position information
    uint32_t current_position_ms;
    uint32_t total_duration_ms;
    
    // Monitor thread
    pthread_t monitor_thread;
    int monitor_running;
    
    // WAV specific information (WAV format only)
    wav_s wav;
    int fd;  // File descriptor for WAV format only
    
    // Compatibility fields
    int seek;
    uint32_t seek_position;
    uint32_t file_position;
    pthread_t pid;
} audioctl_s;

/*********************
 * GLOBAL PROTOTYPES
 *********************/

/**
 * Detect audio file format
 * @param path Audio file path
 * @return Audio format (AUDIO_FORMAT_*)
 */
int audio_ctl_detect_format(const char *path);

/**
 * Initialize audio controller (using NxPlayer)
 * @param path Audio file path
 * @return Controller pointer on success, NULL on failure
 */
audioctl_s *audio_ctl_init_nxaudio(const char *path);

/**
 * Start audio playback
 * @param ctl Audio controller pointer
 * @return 0 on success, other values on failure
 */
int audio_ctl_start(audioctl_s *ctl);

/**
 * Pause playback
 * @param ctl Audio controller pointer
 * @return 0 on success, other values on failure
 */
int audio_ctl_pause(audioctl_s *ctl);

/**
 * @brief Resume playback
 * @param ctl Audio controller pointer
 * @return 0 on success, other values on failure
 */
int audio_ctl_resume(audioctl_s *ctl);

/**
 * @brief Stop playback
 * @param ctl Audio controller pointer
 * @return 0 on success, other values on failure
 */
int audio_ctl_stop(audioctl_s *ctl);

/**
 * @brief Set volume
 * @param ctl Audio controller pointer
 * @param vol Volume (0-100)
 * @return 0 on success, other values on failure
 */
int audio_ctl_set_volume(audioctl_s *ctl, uint16_t vol);

/**
 * @brief Get current playback position (seconds)
 * @param ctl Audio controller pointer
 * @return Playback position (seconds)
 */
int audio_ctl_get_position(audioctl_s *ctl);

/**
 * @brief Seek to specified position
 * @param ctl Audio controller pointer
 * @param ms Target position (milliseconds)
 * @return 0 on success, other values on failure
 */
int audio_ctl_seek(audioctl_s *ctl, unsigned ms);

/**
 * @brief Release audio controller
 * @param ctl Audio controller pointer
 * @return 0 on success, other values on failure
 */
int audio_ctl_uninit_nxaudio(audioctl_s *ctl);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* AUDIO_CTL_H */
