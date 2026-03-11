/****************************************************************************
 * packages/demos/mimo/mimo_main.c
 *
 * Copyright (C) 2024 Xiaomi Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mimo.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define INPUT_BUF_SIZE  1024

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void print_banner(void)
{
  printf("\n");
  printf("  MiMo - Xiaomi AI Assistant\n");
  printf("  Model: %s\n", MIMO_MODEL);
  printf("  Thinking: %s\n",
#ifdef CONFIG_MIMO_ENABLE_THINKING
         "on"
#else
         "off"
#endif
         );
  printf("  Type 'quit' to exit.\n");
  printf("\n");
}

static int run_interactive(void)
{
  char input[INPUT_BUF_SIZE];
  char *response;
  int ret;

  print_banner();

  while (1)
    {
      printf("You> ");
      fflush(stdout);

      if (!fgets(input, sizeof(input), stdin))
        {
          break;
        }

      /* Strip newline */

      size_t len = strlen(input);
      if (len > 0 && input[len - 1] == '\n')
        {
          input[len - 1] = '\0';
        }

      if (strlen(input) == 0)
        {
          continue;
        }

      if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0)
        {
          break;
        }

      ret = mimo_agent_chat(input, &response);
      if (ret == 0 && response)
        {
          printf("MiMo> %s\n\n", response);
          free(response);
        }
      else
        {
          printf("MiMo> [error]\n\n");
        }
    }

  return 0;
}

static int run_oneshot(const char *message)
{
  char *response;
  int ret;

  ret = mimo_agent_chat(message, &response);
  if (ret == 0 && response)
    {
      printf("%s\n", response);
      free(response);
    }

  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  int ret;

  /* Initialize provider */

  ret = mimo_provider_init(MIMO_API_KEY, MIMO_API_BASE_URL);
  if (ret < 0)
    {
      fprintf(stderr, "[mimo] Provider init failed. "
              "Check API key configuration.\n");
      return ret;
    }

  /* Initialize agent */

  ret = mimo_agent_init();
  if (ret < 0)
    {
      fprintf(stderr, "[mimo] Agent init failed\n");
      return ret;
    }

  /* Run mode: oneshot or interactive */

  if (argc > 1)
    {
      /* Concatenate all args as the message */

      char message[INPUT_BUF_SIZE];
      message[0] = '\0';

      int i;
      for (i = 1; i < argc; i++)
        {
          if (i > 1)
            {
              strlcat(message, " ", sizeof(message));
            }

          strlcat(message, argv[i], sizeof(message));
        }

      ret = run_oneshot(message);
    }
  else
    {
      ret = run_interactive();
    }

  mimo_provider_destroy();
  return ret;
}
