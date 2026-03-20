#include <assert.h>
#include <stdlib.h>

#include "c_utils/hal_util_mem.h"

/* Compatibility wrapper for newlib's __assert_func used by SDK */

void __assert_func(const char *file, int line,
                   const char *func, const char *expr)
{
  (void)func;
  __assert(file, line, expr);
}

void *util_malloc(int32_t size)
{
  if (size <= 0)
    {
      return NULL;
    }

  return malloc((size_t)size);
}

void util_free(void *ptr)
{
  if (ptr == NULL)
    {
      return;
    }

  free(ptr);
}

void *util_realloc(void *ptr, int32_t size)
{
  if (size <= 0)
    {
      free(ptr);
      return NULL;
    }

  return realloc(ptr, (size_t)size);
}
