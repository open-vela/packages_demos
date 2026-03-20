#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>

#include "c_utils/hal_util_storage.h"

#define BAILIAN_STORAGE_DIR  "/data/bailian"
#define BAILIAN_STORAGE_FILE "/data/bailian/storage.bin"

static int ensure_storage_dir(void)
{
  struct stat st;
  if (stat(BAILIAN_STORAGE_DIR, &st) == 0)
    {
      return 0;  /* Directory exists */
    }
  if (mkdir(BAILIAN_STORAGE_DIR, 0755) == 0)
    {
      sync();  /* Ensure directory is persisted */
      return 0;
    }
  if (errno == EEXIST)
    {
      return 0;
    }
  return -errno;
}

int32_t util_storage_erase(void)
{
  int ret;
  ret = ensure_storage_dir();
  if (ret < 0)
    {
      return UTIL_ERR_IO;
    }
  ret = unlink(BAILIAN_STORAGE_FILE);
  if (ret == 0 || errno == ENOENT)
    {
      sync();
      return UTIL_SUCCESS;
    }
  return UTIL_ERR_IO;
}

int32_t util_storage_storage(uint8_t *data, uint32_t size)
{
  int fd;
  ssize_t written = 0;
  ssize_t ret;
  if (data == NULL || size == 0)
    {
      return UTIL_ERR_INVALID_PARAM;
    }
  ret = ensure_storage_dir();
  if (ret < 0)
    {
      return UTIL_ERR_IO;
    }
  fd = open(BAILIAN_STORAGE_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0)
    {
      return UTIL_ERR_IO;
    }
  while ((uint32_t)written < size)
    {
      ret = write(fd, data + written, size - (uint32_t)written);
      if (ret < 0)
        {
          close(fd);
          return UTIL_ERR_IO;
        }
      written += ret;
    }
  fsync(fd);
  close(fd);
  return UTIL_SUCCESS;
}

int32_t util_storage_load(uint8_t *data, uint32_t size)
{
  int fd;
  ssize_t total = 0;
  ssize_t ret;
  if (data == NULL || size == 0)
    {
      return UTIL_ERR_INVALID_PARAM;
    }
  fd = open(BAILIAN_STORAGE_FILE, O_RDONLY);
  if (fd < 0)
    {
      if (errno == ENOENT)
        {
          return UTIL_ERR_NOT_FOUND;
        }
      return UTIL_ERR_IO;
    }
  while ((uint32_t)total < size)
    {
      ret = read(fd, data + total, size - (uint32_t)total);
      if (ret < 0)
        {
          close(fd);
          return UTIL_ERR_IO;
        }
      if (ret == 0)
        {
          close(fd);
          return UTIL_ERR_FAIL;
        }
      total += ret;
    }
  close(fd);
  return UTIL_SUCCESS;
}
