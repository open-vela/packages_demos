/****************************************************************************
 * packages/demos/ai_chat/include/ai_conversation.h
 *
 * Copyright (C) 2020 Xiaomi Corporation
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
 */

#ifndef PACKAGES_AI_INCLUDE_AI_CONVERSATION_H
#define PACKAGES_AI_INCLUDE_AI_CONVERSATION_H

#include <ai_defs.h>
#include <uv.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

typedef void* conversation_handle_t;

typedef enum {
    CONVERSATION_ENGINE_TYPE_VOLC,
} conversation_engine_type;

//TBD:可以改为 bussy 控制整个media的状态
// 播放器状态
// 0: idle
// 1: bussy
typedef enum {
    CONVERSATION_PLAYER_STATUS_IDLE = 0,
    CONVERSATION_PLAYER_STATUS_OPENED = 1,
    CONVERSATION_PLAYER_STATUS_PLAYING = 2,
    CONVERSATION_PLAYER_STATUS_STOPPED = 3,
    CONVERSATION_PLAYER_STATUS_CLOSED = 4,
    CONVERSATION_PLAYER_STATUS_ERROR = 5,
    CONVERSATION_PLAYER_STATUS_PLAYING_MUSIC = 6,

    CONVERSATION_PLAYER_STATUS_XXXX = 100,
} conversation_player_status_t;

typedef struct conversation_init_params {
    int version;
    uv_loop_t* loop;
    const char* api_key;
    bool auto_next_round; // 是否自动进入下一轮对话
    conversation_engine_type engine_type;
} conversation_init_params_t;

//TBD:命名要统一规范
typedef enum {
    conversation_event_unknown,
    conversation_event_start,
    conversation_event_stop,
    conversation_event_complete,
    conversation_event_input_text,
    conversation_event_response_audio_start,
    conversation_event_response_audio,
    conversation_event_response_text,
    conversation_event_mcp_request,
    conversation_event_error,
} conversation_event_t;

typedef enum {
    conversation_error_success = 0,
    conversation_error_unknown,
    conversation_error_network,
    conversation_error_server,
    conversation_error_cancelled,
} conversation_error_t;

typedef struct conversation_result {
    char* result;
    int len;
    conversation_error_t error_code;
} conversation_result_t;

typedef struct mcp_param_def_s {
    const char *name;
    const char *description;
    const char *type;
    bool required;
} mcp_param_def_t;

typedef struct mcp_tool_def_s {
    const char *name;
    const char *description;
    const mcp_param_def_t *parameters;
    size_t param_count;
} mcp_tool_def_t;

typedef void (*conversation_callback_t)(conversation_event_t event, 
                                       const conversation_result_t* result, 
                                       void* cookie);

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/**
 * @brief Create ai conversation engine.
 * @param[in] param conversation init params
 * @return conversation engine handle
 */
conversation_handle_t ai_conversation_create_engine(const conversation_init_params_t* param);

/**
 * @brief Set conversation engine listener.
 * @param[in] handle conversation handle
 * @param[in] callback conversation event callback
 * @param[in] cookie callback cookie
 * @return 0 on success, otherwise failed
 */
int ai_conversation_set_listener(conversation_handle_t handle, 
                                conversation_callback_t callback, 
                                void* cookie);

/**
 * @brief Start conversation engine.
 * @param[in] handle conversation handle
 * @param[in] audio_info conversation audio info
 * @return 0 on success, otherwise failed
 */
int ai_conversation_start(conversation_handle_t handle);

/**
 * @brief Finish current audio input.
 * @param[in] handle conversation handle
 * @return 0 on success, otherwise failed
 */
int ai_conversation_finish(conversation_handle_t handle);

/**
 * @brief Cancel current conversation.
 * @param[in] handle conversation handle
 * @return 0 on success, otherwise failed
 */
int ai_conversation_cancel(conversation_handle_t handle);

/**
 * @brief Check if conversation engine is busy.
 * @param[in] handle conversation handle
 * @return 1 if busy, 0 if idle, negative if error
 */
int ai_conversation_is_busy(conversation_handle_t handle);

/**
 * @brief Close conversation engine.
 * @param[in] handle conversation handle
 * @return 0 on success, otherwise failed
 */
int ai_conversation_close(conversation_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* PACKAGES_AI_INCLUDE_AI_CONVERSATION_H */ 
