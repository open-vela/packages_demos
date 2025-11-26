/****************************************************************************
 * packages/demos/ai_chat/src/conversation/plugin/ai_conversation_plugin.c
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

#include <stdlib.h>

#include "ai_log.h"
#include "ai_conversation_plugin.h"

void* conversation_plugin_init(conversation_engine_plugin_t* plugin,
                              const conversation_engine_init_params_t* param)
{
    if (!plugin || !param) {
        return NULL;
    }
    int ret;
    void* priv_ctx;

    priv_ctx = zalloc(plugin->priv_size);
    if (!priv_ctx) {
        return NULL;
    }

    if (plugin->init) {
        ret = plugin->init(priv_ctx, param);
        if (ret < 0) {
            CON_ERR("AI plugin:%s init failed: %d", plugin->name, ret);
            free(priv_ctx);
            return NULL;
        }
    }

    return priv_ctx;
}

void conversation_plugin_uninit(conversation_engine_plugin_t* plugin, void* engine, int sync)
{
    if (plugin->uninit && engine) {
        CON_INFO("AI plugin:%s uninit", plugin->name);
        plugin->uninit(engine);
    }

    if (sync) {
        free(engine);
        engine = NULL;
    }
}

void conversation_plugin_destroy(conversation_engine_plugin_t* plugin, void* engine)
{
    if (engine) {
        free(engine);
        engine = NULL;
    }
}