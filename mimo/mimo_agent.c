/****************************************************************************
 * packages/demos/mimo/mimo_agent.c
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

#define AGENT_MAX_TURNS  10

/* MiMo recommended system prompts */

#define MIMO_SYSTEM_PROMPT_EN \
  "You are MiMo, an AI assistant developed by Xiaomi. " \
  "You are helpful, concise, and efficient."

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct mimo_message_s g_history[MIMO_MAX_MESSAGES];
static int g_history_count = 0;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void history_free_entry(struct mimo_message_s *m)
{
  free(m->content);
  free(m->tool_call_id);
  free(m->tool_name);
  free(m->reasoning_content);
  m->content = NULL;
  m->tool_call_id = NULL;
  m->tool_name = NULL;
  m->reasoning_content = NULL;
}

static void history_compact(void)
{
  int keep = MIMO_MAX_MESSAGES / 2;
  int drop = g_history_count - keep;
  int i;

  for (i = 0; i < drop; i++)
    {
      history_free_entry(&g_history[i]);
    }

  memmove(g_history, g_history + drop,
          keep * sizeof(struct mimo_message_s));
  g_history_count = keep;

  printf("[mimo:agent] Compacted history to %d messages\n", keep);
}

static void history_add(enum mimo_role_e role, const char *content,
                        const char *tool_call_id,
                        const char *reasoning_content)
{
  if (g_history_count >= MIMO_MAX_MESSAGES)
    {
      history_compact();
    }

  struct mimo_message_s *m = &g_history[g_history_count++];
  m->role = role;
  m->content = content ? strdup(content) : NULL;
  m->tool_call_id = tool_call_id ? strdup(tool_call_id) : NULL;
  m->tool_name = NULL;
  m->reasoning_content = reasoning_content ? strdup(reasoning_content)
                                           : NULL;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int mimo_agent_init(void)
{
  g_history_count = 0;
  printf("[mimo:agent] Initialized (max_turns=%d, max_history=%d)\n",
         AGENT_MAX_TURNS, MIMO_MAX_MESSAGES);
  return 0;
}

int mimo_agent_chat(const char *user_input, char **response)
{
  struct mimo_chat_req_s req;
  struct mimo_chat_resp_s resp;
  int turn;
  int ret;

  *response = NULL;

  /* Add user message to history */

  history_add(MIMO_ROLE_USER, user_input, NULL, NULL);

  for (turn = 0; turn < AGENT_MAX_TURNS; turn++)
    {
      /* Build request */

      memset(&req, 0, sizeof(req));
      req.model         = MIMO_MODEL;
      req.system_prompt  = MIMO_SYSTEM_PROMPT_EN;
      req.messages       = g_history;
      req.message_count  = g_history_count;
      req.tools_json     = NULL;  /* TODO: wire tool schemas */
      req.temperature    = MIMO_TEMPERATURE;

      /* Call provider */

      memset(&resp, 0, sizeof(resp));
      ret = mimo_provider_chat(&req, &resp);
      if (ret < 0)
        {
          fprintf(stderr, "[mimo:agent] Provider call failed\n");
          history_add(MIMO_ROLE_ASSISTANT,
                      "Sorry, I encountered an error.", NULL, NULL);
          break;
        }

      if (resp.is_tool_use && resp.tool_name)
        {
          /* Tool use: add assistant message with reasoning_content,
           * execute tool, add result, and continue loop.
           */

          printf("[mimo:agent] Tool call: %s\n", resp.tool_name);

          history_add(MIMO_ROLE_ASSISTANT, resp.content,
                      resp.tool_call_id, resp.reasoning_content);

          /* TODO: Execute tool and get result.
           * For now, return a placeholder error.
           */

          history_add(MIMO_ROLE_TOOL,
                      "{\"error\":\"tool execution not implemented\"}",
                      resp.tool_call_id, NULL);

          mimo_chat_resp_free(&resp);
          continue;
        }

      /* Final response */

      if (resp.content)
        {
          history_add(MIMO_ROLE_ASSISTANT, resp.content, NULL,
                      resp.reasoning_content);
          *response = strdup(resp.content);
        }

      mimo_chat_resp_free(&resp);
      break;
    }

  if (!*response)
    {
      *response = strdup("[MiMo] No response generated.");
    }

  return 0;
}
