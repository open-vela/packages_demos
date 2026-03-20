#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "c_utils/hal_util_mutex.h"
#include "c_utils/c_utils.h"

static int mutex_trylock_with_timeout(pthread_mutex_t *mutex, int32_t timeout)
{
  int32_t elapsed = 0;

  while (elapsed < timeout)
    {
      int ret = pthread_mutex_trylock(mutex);
      if (ret == 0)
        {
          return UTIL_SUCCESS;
        }

      if (ret != EBUSY)
        {
          return UTIL_ERR_FAIL;
        }

      usleep(1000);
      elapsed += 1;
    }

  return UTIL_ERR_TIMEOUT;
}

util_mutex_t *util_mutex_create(void)
{
  util_mutex_t *mutex;
  pthread_mutex_t *handle;

  mutex = (util_mutex_t *)malloc(sizeof(util_mutex_t));
  if (mutex == NULL)
    {
      return NULL;
    }

  handle = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
  if (handle == NULL)
    {
      free(mutex);
      return NULL;
    }

  if (pthread_mutex_init(handle, NULL) != 0)
    {
      free(handle);
      free(mutex);
      return NULL;
    }

  mutex->mutex_handle = handle;
  return mutex;
}

void util_mutex_delete(util_mutex_t *mutex)
{
  pthread_mutex_t *handle;

  if (mutex == NULL)
    {
      return;
    }

  handle = (pthread_mutex_t *)mutex->mutex_handle;
  if (handle != NULL)
    {
      pthread_mutex_destroy(handle);
      free(handle);
    }

  free(mutex);
}

int32_t util_mutex_lock(util_mutex_t *mutex, int32_t timeout)
{
  pthread_mutex_t *handle;
  int ret;

  if (mutex == NULL || mutex->mutex_handle == NULL)
    {
      return UTIL_ERR_INVALID_PARAM;
    }

  handle = (pthread_mutex_t *)mutex->mutex_handle;

  if (timeout == MUTEX_WAIT_FOREVER)
    {
      ret = pthread_mutex_lock(handle);
      return ret == 0 ? UTIL_SUCCESS : UTIL_ERR_FAIL;
    }

  if (timeout <= 0)
    {
      ret = pthread_mutex_trylock(handle);
      if (ret == 0)
        {
          return UTIL_SUCCESS;
        }
      return ret == EBUSY ? UTIL_ERR_TIMEOUT : UTIL_ERR_FAIL;
    }

  return mutex_trylock_with_timeout(handle, timeout);
}

int32_t util_mutex_unlock(util_mutex_t *mutex)
{
  pthread_mutex_t *handle;
  int ret;

  if (mutex == NULL || mutex->mutex_handle == NULL)
    {
      return UTIL_ERR_INVALID_PARAM;
    }

  handle = (pthread_mutex_t *)mutex->mutex_handle;
  ret = pthread_mutex_unlock(handle);
  return ret == 0 ? UTIL_SUCCESS : UTIL_ERR_FAIL;
}
