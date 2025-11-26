/****************************************************************************
 * packages/demos/ai_chat/src/conversation/plugin/ai_conversation_plugin.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef PACKAGES_AI_CONVERSATION_PLUGIN_H_
#define PACKAGES_AI_CONVERSATION_PLUGIN_H_

#include <uv.h>
#include "ai_conversation_defs.h"

typedef struct conversation_engine_plugin_s {
    const char* name;
    int priv_size;
    int (*init)(void* engine, const conversation_engine_init_params_t* param);
    int (*uninit)(void* engine);
    int (*event_cb)(void* engine, conversation_engine_callback_t callback, void* cookie);
    int (*start)(void* engine);
    int (*write_audio)(void* engine, const char* data, int len);
    int (*finish)(void* engine);
    int (*cancel)(void* engine);
    int (*mcp_response)(void* engine, char* result);
    conversation_engine_env_params_t* (*get_env)(void* engine);
} conversation_engine_plugin_t;

void* conversation_plugin_init(conversation_engine_plugin_t* plugin,
                              const conversation_engine_init_params_t* param);
void conversation_plugin_uninit(conversation_engine_plugin_t* plugin, void* engine, int sync);
void conversation_plugin_destroy(conversation_engine_plugin_t* plugin, void* engine);

#endif // PACKAGES_AI_CONVERSATION_PLUGIN_H_