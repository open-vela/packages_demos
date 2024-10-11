/*********************
 *      INCLUDES
 *********************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "audio_ctl.h"

#include <audioutils/nxaudio.h>

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void app_dequeue_cb(unsigned long arg,
                           FAR struct ap_buffer_s *apb);
static void app_complete_cb(unsigned long arg);
static void app_user_cb(unsigned long arg,
                        FAR struct audio_msg_s *msg, FAR bool *running);

/**********************
 *  STATIC VARIABLES
 **********************/

static struct nxaudio_callbacks_s cbs =
{
  app_dequeue_cb,
  app_complete_cb,
  app_user_cb
};

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void app_dequeue_cb(unsigned long arg, FAR struct ap_buffer_s *apb)
{
    FAR audioctl_s *ctl = (FAR audioctl_s *)(uintptr_t)arg;

    if (!apb)
    {
        return;
    }

    if (ctl->seek) {
        lseek(ctl->fd, ctl->seek_position, SEEK_SET);
        ctl->file_position = ctl->seek_position;
        ctl->seek = false;
    }

    apb->nbytes = read(ctl->fd, apb->samp, apb->nmaxbytes);
    apb->curbyte = 0;
    apb->flags = 0;

    while (0 < apb->nbytes && apb->nbytes < apb->nmaxbytes)
    {
        int n = apb->nmaxbytes - apb->nbytes;
        int ret = read(ctl->fd, &apb->samp[apb->nbytes], n);

        if (0 >= ret)
        {
            break;
        }
        apb->nbytes += ret;
    }

    if (apb->nbytes < apb->nmaxbytes)
    {
        close(ctl->fd);
        ctl->fd = -1;

        return ;
    }

    ctl->file_position += apb->nbytes;

    nxaudio_enqbuffer(&ctl->nxaudio, apb);
}

static void app_complete_cb(unsigned long arg)
{
    /* Do nothing.. */

    printf("Audio loop is Done\n");
}

static void app_user_cb(unsigned long arg,
                        FAR struct audio_msg_s *msg, FAR bool *running)
{
    /* Do nothing.. */
}

static FAR void *audio_loop_thread(pthread_addr_t arg)
{
    FAR audioctl_s *ctl = (FAR audioctl_s *)arg;

    nxaudio_start(&ctl->nxaudio);
    nxaudio_msgloop(&ctl->nxaudio, &cbs,
                    (unsigned long)(uintptr_t)ctl);

    return NULL;
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

FAR audioctl_s *audio_ctl_init_nxaudio(FAR const char *arg)
{
    FAR audioctl_s *ctl;
    int ret;
    int i;

    ctl = (FAR audioctl_s *)malloc(sizeof(audioctl_s));
    if(ctl == NULL)
    {
        return NULL;
    }

    ctl->seek = false;
    ctl->seek_position = 0;
    ctl->file_position = 0;

    ctl->fd = open(arg, O_RDONLY);
    if (ctl->fd < 0) {
       printf("can't open audio file\n");
       return NULL;
    }

    read(ctl->fd, &ctl->wav, sizeof(ctl->wav));

    ret = init_nxaudio(&ctl->nxaudio, ctl->wav.fmt.samplerate,
                       ctl->wav.fmt.bitspersample,
                       ctl->wav.fmt.numchannels);
    if (ret < 0)
    {
        printf("init_nxaudio() return with error!!\n");
        return NULL;
    }

    for (i = 0; i < ctl->nxaudio.abufnum; i++)
    {
        app_dequeue_cb((unsigned long)ctl, ctl->nxaudio.abufs[i]);
    }

    ctl->state = AUDIO_CTL_STATE_INIT;

    return ctl;
}

int audio_ctl_start(FAR audioctl_s *ctl)
{
    if (ctl == NULL)
        return -EINVAL;

    if (ctl->state != AUDIO_CTL_STATE_INIT && ctl->state != AUDIO_CTL_STATE_PAUSE)
    {
        return -1;
    }

    ctl->state = AUDIO_CTL_STATE_START;

    pthread_attr_t tattr;
    struct sched_param sparam;

    pthread_attr_init(&tattr);
    sparam.sched_priority = sched_get_priority_max(SCHED_FIFO) - 9;
    pthread_attr_setschedparam(&tattr, &sparam);
    pthread_attr_setstacksize(&tattr, 4096);

    pthread_create(&ctl->pid, &tattr, audio_loop_thread,
                                (pthread_addr_t)ctl);

    pthread_attr_destroy(&tattr);
    pthread_setname_np(ctl->pid, "audioctl_thread");

    return 0;
}

int audio_ctl_pause(FAR audioctl_s *ctl)
{
    if (ctl == NULL)
        return -EINVAL;

    if (ctl->state != AUDIO_CTL_STATE_START)
    {
        return -1;
    }

    ctl->state = AUDIO_CTL_STATE_PAUSE;

    return nxaudio_pause(&ctl->nxaudio);
}

int audio_ctl_resume(FAR audioctl_s *ctl)
{
    if (ctl == NULL)
        return -EINVAL;

    if (ctl->state != AUDIO_CTL_STATE_PAUSE)
    {
        return -1;
    }

    ctl->state = AUDIO_CTL_STATE_START;

    return nxaudio_resume(&ctl->nxaudio);
}

int audio_ctl_seek(FAR audioctl_s *ctl, unsigned ms)
{
    if (ctl == NULL)
        return -EINVAL;

    ctl->seek_position = ms * ctl->wav.fmt.samplerate * ctl->wav.fmt.bitspersample * ctl->wav.fmt.numchannels / 8;
    ctl->seek = true;

    return 0;
}

int audio_ctl_stop(FAR audioctl_s *ctl)
{
    if (ctl == NULL)
        return -EINVAL;

    if (ctl->state != AUDIO_CTL_STATE_PAUSE && ctl->state != AUDIO_CTL_STATE_START)
    {
        return -1;
    }

    ctl->state = AUDIO_CTL_STATE_STOP;

    nxaudio_stop(&ctl->nxaudio);

    if (ctl->pid > 0)
    {
        pthread_join(ctl->pid, NULL);
    }

    return 0;
}

int audio_ctl_set_volume(FAR audioctl_s *ctl, uint16_t vol)
{
    if (ctl == NULL)
        return -EINVAL;

    return nxaudio_setvolume(&ctl->nxaudio, vol);
}

int audio_ctl_get_position(FAR audioctl_s *ctl)
{
    if (ctl == NULL)
        return -EINVAL;

    return ctl->file_position / (ctl->wav.fmt.bitspersample * ctl->wav.fmt.numchannels * ctl->wav.fmt.samplerate / 8);
}

int audio_ctl_uninit_nxaudio(FAR audioctl_s *ctl)
{
    if (ctl == NULL)
        return -EINVAL;

    if (ctl->state == AUDIO_CTL_STATE_NOP)
    {
        return 0;
    }

    if (ctl->fd > 0)
    {
        close(ctl->fd);
        ctl->fd = -1;
    }

    fin_nxaudio(&ctl->nxaudio);

    free(ctl);

    return 0;
}
