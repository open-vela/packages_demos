#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "c_utils/hal_util_time.h"

int64_t util_now_ms(void)
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
    {
      return 0;
    }

  return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void util_msleep(uint32_t ms)
{
  if (ms == 0)
    {
      return;
    }

  usleep((useconds_t)ms * 1000);
}

int64_t util_get_timestamp(void)
{
  struct timespec ts;

  if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
    {
      return 0;
    }

  return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

int64_t util_get_timestamp_ms(void)
{
  return util_get_timestamp();
}

uint8_t util_timestamp_inited(void)
{
  return 1;
}
