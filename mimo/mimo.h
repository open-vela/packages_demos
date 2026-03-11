/****************************************************************************
 * packages/demos/mimo/mimo.h
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

#ifndef __DEMOS_MIMO_H
#define __DEMOS_MIMO_H

#include <nuttx/config.h>
#include <stdint.h>
#include <stdbool.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define MIMO_MAX_MESSAGES     32
#define MIMO_MAX_RESPONSE     (32 * 1024)
#define MIMO_MAX_TOOL_RESULT  (8 * 1024)
#define MIMO_MAX_URL_LEN      256
#define MIMO_MAX_TOOLS        16

/* Configuration from Kconfig */

#define MIMO_API_KEY       CONFIG_MIMO_API_KEY
#define MIMO_MODEL         CONFIG_MIMO_MODEL
#define MIMO_API_BASE_URL  CONFIG_MIMO_API_BASE_URL
#define MIMO_TEMPERATURE   CONFIG_MIMO_TEMPERATURE

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Message roles */

enum mimo_role_e
{
  MIMO_ROLE_SYSTEM = 0,
  MIMO_ROLE_USER,
  MIMO_ROLE_ASSISTANT,
  MIMO_ROLE_TOOL
};

/* A single message in conversation */

struct mimo_message_s
{
  enum mimo_role_e  role;
  char             *content;            /* Heap-allocated text */
  char             *tool_call_id;       /* Non-NULL if role == TOOL */
  char             *tool_name;          /* Non-NULL if assistant requested tool */
  char             *reasoning_content;  /* Thinking mode reasoning (may be NULL) */
};

/* Chat completion request */

struct mimo_chat_req_s
{
  const char            *model;
  const char            *system_prompt;
  struct mimo_message_s *messages;
  int                    message_count;
  const char            *tools_json;   /* JSON array of tool schemas, or NULL */
  int                    temperature;  /* x100, e.g. 80 = 0.8 */
};

/* Chat completion response */

struct mimo_chat_resp_s
{
  char *content;            /* Assistant text (heap-allocated) */
  char *tool_call_id;       /* Non-NULL if tool_use stop reason */
  char *tool_name;          /* Tool name to invoke */
  char *tool_input;         /* Tool input JSON string */
  char *reasoning_content;  /* Thinking mode reasoning (heap-allocated) */
  bool  is_tool_use;        /* true if the model wants to call a tool */
};

/****************************************************************************
 * Public Function Prototypes - Provider
 ****************************************************************************/

int  mimo_provider_init(const char *api_key, const char *base_url);
int  mimo_provider_chat(const struct mimo_chat_req_s *req,
                        struct mimo_chat_resp_s *resp);
void mimo_chat_resp_free(struct mimo_chat_resp_s *resp);
void mimo_provider_destroy(void);

/****************************************************************************
 * Public Function Prototypes - Agent
 ****************************************************************************/

int  mimo_agent_init(void);
int  mimo_agent_chat(const char *user_input, char **response);

#endif /* __DEMOS_MIMO_H */
