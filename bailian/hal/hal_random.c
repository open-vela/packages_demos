#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "c_utils/hal_util_random.h"

static uint32_t g_prng_seed = 1;
static uint8_t g_prng_initialized = 0;

int32_t util_random_init(uint32_t seed)
{
  g_prng_seed = seed;
  g_prng_initialized = 1;
  srand(seed);
  return 0;
}

uint32_t util_random(void)
{
  uint32_t value = 0;
  int fd;
  ssize_t ret;
  /* Try hardware random first */
  fd = open("/dev/urandom", O_RDONLY);
  if (fd >= 0)
    {
      ret = read(fd, &value, sizeof(value));
      close(fd);
      if (ret == (ssize_t)sizeof(value))
        {
          return value;
        }
    }
  /* Fallback to PRNG */
  if (!g_prng_initialized)
    {
      util_random_init(1);
    }
  return (uint32_t)rand();
}
