/****************************************************************************
 * packages/demos/ai_chat/src/conversation/volc/ai_volc_conversation.c
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <json-c/json_object.h>
#include <json-c/json_object_iterator.h>
#include <json-c/json_types.h>
#include <json_object.h>
#include <json_tokener.h>
#include <libwebsockets.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include "ai_log.h"
#include "ai_conversation_plugin.h"
#include "ai_conversation.h"
#include "ai_circular_buffer.h"

#define VOLC_API_KEY CONFIG_VOLC_PULGIN_API_KEY
#define VOLC_URL "wss://ai-gateway.vei.volces.com/v1/realtime"
#define VOLC_HOST "ai-gateway.vei.volces.com"
#define VOLC_PATH "/v1/realtime"
#define VOLC_CLIENT_PROTOCOL_NAME ""

#define VOLC_BUFFER_MAX_SIZE 128 * 1024
#define VOLC_LOOP_INTERVAL 1000 // microseconds
#define VOLC_PING_INTERVAL 100000000 // 100 seconds if server 120s no message, consider it as timeout
#define VOLC_RESPONSE_TIMEOUT 20000000 // 20 seconds if server 20s no response, consider it as error

/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef enum {
    VOLC_STATE_DISCONNECTED,
    VOLC_STATE_CONNECTING,
    VOLC_STATE_CONNECTED,
    VOLC_STATE_SESSION_CREATED,
    VOLC_STATE_LISTENING,
    VOLC_STATE_PROCESSING,
    VOLC_STATE_SPEAKING,
    VOLC_STATE_ERROR
} volc_conversation_state_t;

typedef struct volc_conversation_engine {
    // WebSocket connection
    struct lws_context* lws_context;
    struct lws* wsi;

    // State management
    volc_conversation_state_t state;  //TBD 这个状态需要思考一下
    conversation_engine_callback_t event_callback;
    void* event_cookie;
    bool is_created;    // 是否已经创建会话
    bool is_finished;  // 用户音频输入是否结束
    bool is_closed;    // 整个连接是否关闭
    bool is_running;

    // Thread and event loop
    pthread_t thread;
    // TBD:删除 sem_t
    sem_t sem;
    void* opaque;

    pthread_rwlock_t response_lock;

    // Configuration
    conversation_engine_init_params_t config;

    // Authentication
    char* api_key;

    // Send buffer
    ai_circular_buffer_t send_buffer;
    char* send_buffer_data;

    // Session data
    char* session_id;
    char* current_response_id; 
    //TBD:删除 response_id session_id
    //    理由，目前没有用到，只是凭空增加代码复杂度

    char* call_tool_name;
    char* call_tool_id;

    // Environment
    conversation_engine_env_params_t env;

    uint64_t last_ping_time;
    bool ping_pending;

    uint64_t last_response_time;
    bool response_pending;

} volc_conversation_engine_t;

static const mcp_param_def_t g_turn_light_params[] = {
    { "properties", "string", "灯所在位置", true },
};

static const mcp_param_def_t g_music_play_params[] = {
    { "properties", "string", "歌曲名称；若为空则播放“随机音乐”", true },
};

static const mcp_param_def_t g_adjust_volume_params[] = {
    { "properties", "string", "0-100 的音量值", true },
};

static const mcp_param_def_t g_launch_app_params[] = {
    { "properties", "string", "应用名称", true },
};

static const mcp_param_def_t g_increase_or_decrease_volume_params[] = {
    { "properties", "string", "增加或减少音量，只支持输入“增加”或“减少”", true },
};

static const mcp_tool_def_t g_mcp_tools[] = {
    { "turn_light_on", "控制灯亮度",
       g_turn_light_params,
      (sizeof(g_turn_light_params) / sizeof((g_turn_light_params)[0])) },
    { "music_play", "播放音乐",
       g_music_play_params,
      (sizeof(g_music_play_params) / sizeof((g_music_play_params)[0])) },
    { "adjust_volume", "调节系统音量",  // only support in x4b platform
       g_adjust_volume_params,
      (sizeof(g_adjust_volume_params) / sizeof((g_adjust_volume_params)[0])) },
    { "increase_or_decrease_volume", "增加或减少系统音量", // only support in x4b platform
       g_increase_or_decrease_volume_params,
      (sizeof(g_increase_or_decrease_volume_params) / sizeof((g_increase_or_decrease_volume_params)[0])) },
    { "get_volume", "获取系统音量", // only support in x4b platform
       NULL,
        0 },
    { "launch_app", "打开应用",
       g_launch_app_params,
      (sizeof(g_launch_app_params) / sizeof((g_launch_app_params)[0])) }
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int volc_conversation_websocket_callback(struct lws* wsi, enum lws_callback_reasons reason,
                                               void* user, void* in, size_t len);
static int volc_conversation_send_json_message(volc_conversation_engine_t* engine, 
                                               json_object* json_obj);
static int volc_conversation_process_server_message(volc_conversation_engine_t* engine, 
                                                    const char* message);
static void volc_conversation_send_event(volc_conversation_engine_t* engine,
                                        conversation_engine_event_t event,
                                        const char* result, int len,
                                        conversation_engine_error_t error_code);
static int volc_conversation_connect_websocket(volc_conversation_engine_t* volc_engine);
static int volc_conversation_create_thread(volc_conversation_engine_t* engine);
static int volc_conversation_destroy_thread(volc_conversation_engine_t* engine);
static void* volc_conversation_loop_thread(void* arg);
static char* base64_encode(const unsigned char* data, size_t input_length);
static unsigned char* base64_decode(const char* data, size_t input_length, size_t* output_length);
static int volc_conversation_update_session(void* engine);

/****************************************************************************
 * WebSocket Protocol Implementation
 ****************************************************************************/

static struct lws_protocols volc_conversation_protocols[] = {
    {
        .name = VOLC_CLIENT_PROTOCOL_NAME,
        .callback = volc_conversation_websocket_callback,
        .per_session_data_size = 0,
        .rx_buffer_size = VOLC_BUFFER_MAX_SIZE,
    },
    { NULL, NULL, 0, 0 }
};

static int volc_conversation_websocket_callback(struct lws* wsi, enum lws_callback_reasons reason,
                                               void* user, void* in, size_t len)
{
    volc_conversation_engine_t* engine = \
                        (volc_conversation_engine_t*)lws_context_user(lws_get_context(wsi));
    int ret;

    if (!engine) {
        CON_INFO("Engine is NULL for reason: %d", reason);
        return -1;
    }

    CON_INFO("websocket_callback reason: %d, len: %zu", reason, len);

    switch (reason) {
    case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH:
        CON_INFO("conversation_volc Pre-establish filter");
        break;

    case LWS_CALLBACK_WSI_CREATE:
        CON_INFO("conversation_volc WSI created");
        break;

    case LWS_CALLBACK_CLIENT_HTTP_BIND_PROTOCOL:
        CON_INFO("conversation_volc HTTP bind protocol");
        break;

    case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
        {
            CON_INFO("conversation_volc Add header\n");
            unsigned char** headers = (unsigned char**)in;
            unsigned char* end = (*headers) + len;

            // Add necessary headers for authentication
            char auth_header[128];
            snprintf(auth_header, sizeof(auth_header), "Bearer %s", engine->api_key);

            CON_INFO("Adding Authorization header: Bearer %.*s...", 10, engine->api_key);

            ret = lws_add_http_header_by_name(wsi, (unsigned char*)"Authorization:",
                                            (unsigned char*)auth_header,
                                            strlen(auth_header),
                                            headers, end);
            if (ret < 0)
                CON_ERR("Add Authorization token failed\n");

            // Add User-Agent header
            ret = lws_add_http_header_by_name(wsi,
                (unsigned char*)"User-Agent:",
                (unsigned char*)"curl/7.81.0",
                strlen("curl/7.81.0"),
                headers, end);
            if (ret < 0)
                CON_ERR("Add User-Agent failed\n");

            // Add Accept header
            ret = lws_add_http_header_by_name(wsi,
                (unsigned char*)"Accept:",
                (unsigned char*)"*/*",
                strlen("*/*"),
                headers, end);
            if (ret < 0)
                CON_ERR("Add Accept failed\n");
        }
        break;

    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        CON_INFO("conversation_volc Connected to server: %s\n", VOLC_URL);
        engine->state = VOLC_STATE_CONNECTED;
        break;

    case LWS_CALLBACK_CLIENT_RECEIVE:
        CON_INFO("conversation_volc Received message: %.*s", (int)len, (char*)in);
        if (len > 0) {
            char* message = malloc(len + 1);
            memcpy(message, in, len);
            message[len] = '\0';
            volc_conversation_process_server_message(engine, message);
            free(message);
        }
        break;

    case LWS_CALLBACK_CLIENT_WRITEABLE:
        CON_INFO("conversation_volc WebSocket writable");
        if (engine->is_closed) {
            /* Request graceful close on writable */
            lws_close_reason(wsi, LWS_CLOSE_STATUS_NORMAL, NULL, 0);
            return -1; /* trigger close */
        }
        if (ai_circular_buffer_num_items(&engine->send_buffer) > 0) {
            size_t available = ai_circular_buffer_num_items(&engine->send_buffer);
            size_t message_end = 0;
            bool found_complete_message = false;

            for (size_t i = 0; i < available && i < VOLC_BUFFER_MAX_SIZE - LWS_PRE; i++) {
                char temp_char;
                if (ai_circular_buffer_peek(&engine->send_buffer, \
                                            &temp_char, i) == 1) {
                    if (temp_char == '\n') {
                        message_end = i + 1;
                        found_complete_message = true;
                        break;
                    }
                }
            }

            if (found_complete_message && message_end > 0) {

                unsigned char* buffer = malloc(message_end + LWS_PRE);
                if (buffer) {
                    ai_circular_buffer_dequeue_arr(&engine->send_buffer, \
                                                (char*)(buffer + LWS_PRE), message_end);
                    
                    int written = lws_write(wsi, buffer + LWS_PRE, message_end, \
                                    LWS_WRITE_TEXT);
                    free(buffer);

                    if (written < 0) {
                        return -1;
                    }

                    if (ai_circular_buffer_num_items(&engine->send_buffer) > 0) {
                        lws_callback_on_writable(wsi);
                    }
                }
            } else if (available > 0) {
                ai_circular_buffer_clear_arr(&engine->send_buffer, available);
            }
        }
        break;

    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        CON_INFO("WebSocket connection error: %s", in ? (char*)in : "Unknown error");
        engine->state = VOLC_STATE_ERROR;
        const char *result = in ? (char*)in : "Connection error";
        volc_conversation_send_event(engine, conversation_engine_event_error,
                                    result, strlen(result),
                                    conversation_engine_error_network);
        break;

    case LWS_CALLBACK_CLIENT_CLOSED:
        CON_INFO("WebSocket connection closed");
        engine->wsi = NULL;
        engine->state = VOLC_STATE_DISCONNECTED;
        break;

    case LWS_CALLBACK_WSI_DESTROY:
        CON_INFO("WebSocket connection destroyed");
        engine->wsi = NULL;
        break;

    case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS:
        CON_INFO("conversation_volc Loading SSL certs");
        break;

    case LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION:
        CON_INFO("conversation_volc SSL cert verification");
        break;

    case LWS_CALLBACK_CLIENT_HTTP_WRITEABLE:
        CON_INFO("conversation_volc HTTP writeable");
        break;

    case LWS_CALLBACK_CLIENT_HTTP_REDIRECT:
        CON_INFO("conversation_volc HTTP redirect");
        break;

    case LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION:
        CON_INFO("conversation_volc SSL server cert verification");
        return 0;

    case LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY:
        CON_INFO("conversation_volc SSL context requires private key");
        break;

    case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
        CON_INFO("conversation_volc Confirm extension supported");
        break;

    case LWS_CALLBACK_WS_CLIENT_BIND_PROTOCOL:
        CON_INFO("conversation_volc WS client bind protocol");
        break;

    case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
        CON_INFO("conversation_volc Received pong");
        engine->ping_pending = false;
        break;

    default:
        CON_INFO("conversation_volc Default error\n");
            break;
    }

    return 0;
}

/****************************************************************************
 * JSON Message Processing
 ****************************************************************************/

static int volc_conversation_send_json_message(volc_conversation_engine_t* engine,
                                               json_object* json_obj)
{
    if (!engine || !json_obj || engine->is_closed || !engine->wsi) {
        return -EINVAL;
    }

    const char* json_stcircular = json_object_to_json_string(json_obj);
    size_t json_len = strlen(json_stcircular);

    CON_INFO("Sending: %s", json_stcircular);

    if (ai_circular_buffer_is_full(&engine->send_buffer)) {
        CON_INFO("Send buffer full, cleacircular space");
        ai_circular_buffer_clear_arr(&engine->send_buffer, json_len);
    }

    char* message_with_separator = malloc(json_len + 2); // +2 for \n and \0
    if (!message_with_separator) {
        CON_INFO("Failed to allocate memory for message separator");
        return -ENOMEM;
    }
    
    memcpy(message_with_separator, json_stcircular, json_len);
    message_with_separator[json_len] = '\n';
    message_with_separator[json_len + 1] = '\0';

    ai_circular_buffer_queue_arr(&engine->send_buffer, 
                                message_with_separator, json_len + 1);
    free(message_with_separator);

    lws_callback_on_writable(engine->wsi);

    return 0;
}

static int volc_conversation_process_server_message(volc_conversation_engine_t* engine, 
                                                    const char* message)
{
    json_object* json = json_tokener_parse(message);
    if (!json) {
        CON_INFO("Failed to parse JSON message");
        return -1;
    }

    json_object* type_obj;
    if (!json_object_object_get_ex(json, "type", &type_obj)) {
        json_object_put(json);
        return -1;
    }

    const char* type = json_object_get_string(type_obj);

    if (strcmp(type, "session.created") == 0) {
        json_object* session_obj;
        if (json_object_object_get_ex(json, "session", &session_obj)) {
            json_object* id_obj;
            if (json_object_object_get_ex(session_obj, "id", &id_obj)) {
                const char* session_id = json_object_get_string(id_obj);
                if (engine->session_id) {
                    free(engine->session_id);
                }
                engine->session_id = strdup(session_id);
                CON_INFO("Session created with ID: %s", session_id);
            }
        }

        if (!engine->is_created){
            volc_conversation_update_session(engine);
            engine->is_created = true;
        }
        volc_conversation_send_event(engine, conversation_engine_event_start,
                 engine->session_id, strlen(engine->session_id), 
                    conversation_engine_error_success);

    } else if (strcmp(type, "response.output_item.added") == 0) {

        json_object* item_obj = json_object_object_get(json, "item");
        if (!item_obj) {
            json_object_put(json);
            return -1;
        }

        const char* tool_name = \
                json_object_get_string(json_object_object_get(item_obj, "name"));
        const char* call_id = \
                json_object_get_string(json_object_object_get(item_obj, "call_id"));
        json_object* arguments_obj = json_object_object_get(item_obj, "arguments");
        if (!tool_name || !call_id || !arguments_obj) {
            json_object_put(json);
            return -1;
        }
        const char* argument_string = \
                json_object_get_string(arguments_obj);
        if (!argument_string) {
            json_object_put(json);
            return -1;
        }
        CON_INFO("response arguments string: [%s]", argument_string);
        json_object* arg_obj = json_tokener_parse(argument_string);
        const char* arguments = \
                json_object_get_string(json_object_object_get(arg_obj, "properties"));

        CON_INFO("response properties obj: [%s]", arguments);

        //call_tool_id 去除
        if (engine->call_tool_id)
        {
            free(engine->call_tool_id);
        }
        engine->call_tool_id = strdup(call_id);

        char mcp_data[512] = {0};
        snprintf(mcp_data, sizeof(mcp_data), "%s|%s|%s|",\
                    tool_name, call_id, arguments);

        CON_INFO("response.output_item.added call_name: [%s]", mcp_data);

        volc_conversation_send_event(engine, conversation_engine_event_mcp_call,
                            mcp_data, strlen(mcp_data), \
                            conversation_engine_error_success);

    } else if (strcmp(type, "response.function_call_arguments.done") == 0) {

        const char* call_id = \
            json_object_get_string(json_object_object_get(json, "call_id"));

        if (engine->call_tool_id)
        {
            free(engine->call_tool_id);
        }
        engine->call_tool_id = strdup(call_id);

        volc_conversation_send_event(engine, conversation_engine_event_mcp_argument,
                                engine->call_tool_id, strlen(engine->call_tool_id), \
                            conversation_engine_error_success);

    } else if (strcmp(type, "conversation.item.input_audio_transcription.completed") == 0) {
        json_object* transcript_obj;
        if (json_object_object_get_ex(json, "transcript", &transcript_obj)) {
            const char* transcript = json_object_get_string(transcript_obj);
            volc_conversation_send_event(engine, conversation_engine_event_input_text,
                                       transcript, strlen(transcript), 
                                    conversation_engine_error_success);
        }

    } else if (strcmp(type, "response.created") == 0) {
        json_object* response_obj;
        if (json_object_object_get_ex(json, "response", &response_obj)) {
            json_object* id_obj;
            if (json_object_object_get_ex(response_obj, "id", &id_obj)) {
                const char* response_id = json_object_get_string(id_obj);
                if (engine->current_response_id) {
                    free(engine->current_response_id);
                }
                engine->current_response_id = strdup(response_id);
            }
        }

    } else if (strcmp(type, "response.audio_transcript.done") == 0) {
        volc_conversation_send_event(engine, conversation_engine_event_audio_start,
                                       NULL, 0, 
                                    conversation_engine_error_success);
    } else if (strcmp(type, "response.audio.delta") == 0) {
        json_object* delta_obj;
        if (json_object_object_get_ex(json, "delta", &delta_obj)) {
            const char* audio_b64 = json_object_get_string(delta_obj);

            size_t audio_len;
            unsigned char* audio_data = base64_decode(audio_b64, 
                                            strlen(audio_b64),
                                            &audio_len);

            if (audio_data) {
                volc_conversation_send_event(engine, conversation_engine_event_audio,
                                           (char*)audio_data, audio_len, 
                                           conversation_engine_error_success);
                free(audio_data);
            }
        }

    } else if (strcmp(type, "response.audio_transcript.delta") == 0) {
        json_object* delta_obj;
        engine->response_pending = false;
        if (json_object_object_get_ex(json, "delta", &delta_obj)) {
            const char* text_delta = json_object_get_string(delta_obj);
            const char* text = strdup(text_delta);
            if (text) {
                volc_conversation_send_event(engine, conversation_engine_event_text,
                                       text, strlen(text), 
                                       conversation_engine_error_success);
            }
        }

    } else if (strcmp(type, "response.done") == 0) {
        const char* status = "completed";
        json_object* response_obj;
        if (json_object_object_get_ex(json, "response", &response_obj)) {
            json_object* status_obj;
            if (json_object_object_get_ex(response_obj, "status", &status_obj)) {
                status = json_object_get_string(status_obj);
            }
        }

        if (engine->config.auto_next_round) {
            engine->is_finished = false;
            CON_INFO("Auto next round enabled - ready for immediate input");
        } else {
            CON_INFO("Auto next round disabled - call start() for next round");
        }

        if (strcmp(status, "cancelled") == 0) {
            CON_INFO("Response cancelled by client, ready for next conversation round");
            volc_conversation_send_event(engine, conversation_engine_event_complete,
                                    "cancelled", 9, 
                                    conversation_engine_error_cancelled);
        } else {
            CON_INFO("Response complete, ready for next conversation round");
            volc_conversation_send_event(engine, conversation_engine_event_complete,
                                    NULL, 0, 
                                    conversation_engine_error_success);
        }

        if (engine->current_response_id) {
            free(engine->current_response_id);
            engine->current_response_id = NULL;
        }
    } else if (strcmp(type, "conversation.item.created") == 0) {
        json_object* json_response = json_object_new_object();
        json_object_object_add(json_response, "type", \
                            json_object_new_string("response.create"));
        json_object* response_json = json_object_new_object();
        json_object* modalities_json = json_object_new_array();
        json_object_array_add(modalities_json, json_object_new_string("text"));
        json_object_array_add(modalities_json, json_object_new_string("audio"));
        json_object_object_add(response_json, "modalities", modalities_json);
        json_object_object_add(json_response, "response", response_json);

        int ret = volc_conversation_send_json_message(engine, json_response);

        if (ret < 0) {
            CON_ERR("Failed to send response.create to Volc");
            return ret;
        }

        json_object_put(json_response);
    }else if (strcmp(type, "error") == 0) {
        json_object* error_obj;
        const char* error_message = "Unknown error";
        if (json_object_object_get_ex(json, "error", &error_obj)) {
            json_object* message_obj;
            if (json_object_object_get_ex(error_obj, "message", &message_obj)) {
                error_message = json_object_get_string(message_obj);
            }
        }

        engine->state = VOLC_STATE_ERROR;
        volc_conversation_send_event(engine, conversation_engine_event_error,
                                   error_message, strlen(error_message), 
                                   conversation_engine_error_server);
    }

    json_object_put(json);
    return 0;
}

/****************************************************************************
 * Utility Functions
 ****************************************************************************/

static void volc_conversation_send_event(volc_conversation_engine_t* engine,
                                        conversation_engine_event_t event,
                                        const char* result, int len,
                                        conversation_engine_error_t error_code)
{
    if (!engine || !engine->event_callback ) {
        return;
    }

    conversation_engine_result_t engine_result = {
        .result = result,
        .len = len,
        .error_code = error_code
    };

    CON_INFO(" Sending event: event=%d, result_len=%d", event, len);
    engine->event_callback(event, &engine_result, engine->event_cookie);
}

static char* base64_encode(const unsigned char* data, size_t input_length)
{
    static const char encoding_table[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
        'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3',
        '4', '5', '6', '7', '8', '9', '+', '/'
    };

    size_t output_length = 4 * ((input_length + 2) / 3);
    char* encoded_data = malloc(output_length + 1);
    if (!encoded_data) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    static const int mod_table[] = {0, 2, 1};
    for (int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[output_length - 1 - i] = '=';

    encoded_data[output_length] = '\0';
    return encoded_data;
}

static unsigned char* base64_decode(const char* data, size_t input_length, size_t* output_length)
{
    if (input_length % 4 != 0) return NULL;

    static const unsigned char decoding_table[256] = {
        ['A'] = 0,  ['B'] = 1,  ['C'] = 2,  ['D'] = 3,  ['E'] = 4,
        ['F'] = 5,  ['G'] = 6,  ['H'] = 7,  ['I'] = 8,  ['J'] = 9,
        ['K'] = 10, ['L'] = 11, ['M'] = 12, ['N'] = 13, ['O'] = 14,
        ['P'] = 15, ['Q'] = 16, ['R'] = 17, ['S'] = 18, ['T'] = 19,
        ['U'] = 20, ['V'] = 21, ['W'] = 22, ['X'] = 23, ['Y'] = 24,
        ['Z'] = 25, ['a'] = 26, ['b'] = 27, ['c'] = 28, ['d'] = 29,
        ['e'] = 30, ['f'] = 31, ['g'] = 32, ['h'] = 33, ['i'] = 34,
        ['j'] = 35, ['k'] = 36, ['l'] = 37, ['m'] = 38, ['n'] = 39,
        ['o'] = 40, ['p'] = 41, ['q'] = 42, ['r'] = 43, ['s'] = 44,
        ['t'] = 45, ['u'] = 46, ['v'] = 47, ['w'] = 48, ['x'] = 49,
        ['y'] = 50, ['z'] = 51, ['0'] = 52, ['1'] = 53, ['2'] = 54,
        ['3'] = 55, ['4'] = 56, ['5'] = 57, ['6'] = 58, ['7'] = 59,
        ['8'] = 60, ['9'] = 61, ['+'] = 62, ['/'] = 63, ['='] = 0
    };

    size_t out_len = input_length / 4 * 3;
    if (data[input_length - 1] == '=') out_len--;
    if (data[input_length - 2] == '=') out_len--;

    unsigned char* decoded_data = malloc(out_len);
    if (!decoded_data) return NULL;

    size_t i = 0, j = 0;
    while (i < input_length) {
        unsigned char sextet_a = decoding_table[(unsigned char)data[i++]];
        unsigned char sextet_b = decoding_table[(unsigned char)data[i++]];
        unsigned char sextet_c = decoding_table[(unsigned char)data[i++]];
        unsigned char sextet_d = decoding_table[(unsigned char)data[i++]];

        if (sextet_a == 0x80 || sextet_b == 0x80 || sextet_c == 0x80 || sextet_d == 0x80) {
            free(decoded_data);
            return NULL;
        }

        uint32_t triple = (sextet_a << 18) | (sextet_b << 12) | (sextet_c << 6) | sextet_d;

        if (j < out_len) decoded_data[j++] = (triple >> 16) & 0xFF;
        if (j < out_len) decoded_data[j++] = (triple >> 8) & 0xFF;
        if (j < out_len) decoded_data[j++] = triple & 0xFF;
    }

    *output_length = out_len;
    return decoded_data;
}

/****************************************************************************
 * Plugin Interface Implementation
 ****************************************************************************/

static int volc_conversation_init(void* engine, const conversation_engine_init_params_t* param)
{
    volc_conversation_engine_t* volc_engine = (volc_conversation_engine_t*)engine;

    if (!volc_engine || !param) {
        return -EINVAL;
    }

    CON_INFO("Initializing VolcEngine conversation");

    memcpy(&volc_engine->config, param, sizeof(conversation_engine_init_params_t));

    volc_engine->api_key = strdup(VOLC_API_KEY);

    volc_engine->send_buffer_data = malloc(VOLC_BUFFER_MAX_SIZE);
    if (!volc_engine->send_buffer_data) {
        return -ENOMEM;
    }
    ai_circular_buffer_init(&volc_engine->send_buffer, volc_engine->send_buffer_data, \
                            VOLC_BUFFER_MAX_SIZE);

    volc_engine->env.loop = param->loop;
    volc_engine->env.format = "format=s16le:sample_rate=16000:ch_layout=mono";
    volc_engine->env.force_format = 1;

    volc_engine->state = VOLC_STATE_DISCONNECTED;

    volc_engine->is_created = false;
    volc_engine->is_finished = false;
    volc_engine->is_closed = false;
    volc_engine->is_running = false;
    volc_engine->ping_pending = false;
    volc_engine->response_pending = false;
    volc_engine->last_ping_time = 0;
    volc_engine->last_response_time = 0;

    volc_engine->opaque = param->opaque;

    int ret = volc_conversation_create_thread(volc_engine);
    if (ret < 0) {
        CON_ERR("Failed to create thread");
        free(volc_engine->send_buffer_data);
        free(volc_engine->api_key);
        return ret;
    }

    CON_INFO("VolcEngine conversation initialized");
    return 0;
}

static int volc_conversation_uninit(void* engine)
{
    volc_conversation_engine_t* volc_engine = (volc_conversation_engine_t*)engine;

    if (!volc_engine) {
        return -EINVAL;
    }

    CON_INFO("Uninitializing VolcEngine conversation");

    volc_engine->is_created = false;
    volc_engine->is_finished = true;
    volc_engine->is_closed = true;

    volc_conversation_destroy_thread(volc_engine);

    /* Defer actual socket/context close to the loop thread; just wake it up */
    if (volc_engine->lws_context) {
        lws_cancel_service(volc_engine->lws_context);
    }

    if (volc_engine->send_buffer_data) {
        free(volc_engine->send_buffer_data);
    }
    if (volc_engine->session_id) {
        free(volc_engine->session_id);
    }
    if (volc_engine->current_response_id) {
        free(volc_engine->current_response_id);
    }
    if (volc_engine->api_key) {
        free(volc_engine->api_key);
    }

    CON_INFO("VolcEngine conversation uninitialized");
    return 0;
}

static int volc_conversation_event_cb(void* engine, conversation_engine_callback_t callback, \
                                      void* cookie)
{
    volc_conversation_engine_t* volc_engine = (volc_conversation_engine_t*)engine;

    if (!volc_engine) {
        return -EINVAL;
    }

    volc_engine->event_callback = callback;
    volc_engine->event_cookie = cookie;

    return 0;
}

static int volc_conversation_start(void* engine)
{
    volc_conversation_engine_t* volc_engine = (volc_conversation_engine_t*)engine;

    if (!volc_engine) {
        return -EINVAL;
    }

    CON_INFO("Starting VolcEngine conversation");

    volc_engine->is_finished = false;
    CON_INFO("Audio input enabled for new conversation round");

    int ret = volc_conversation_connect_websocket(volc_engine);
    if (ret < 0) {
        CON_ERR("Failed to ensure WebSocket connection");
        return ret;
    }

    return 0;
}

static int volc_conversation_connect_websocket(volc_conversation_engine_t* volc_engine)
{
    CON_INFO("Creating WebSocket connection in thread");

    if (volc_engine->lws_context && volc_engine->wsi) {
        CON_INFO("WebSocket connection already active, reusing existing connection");
        return 0;
    }

    if (volc_engine->lws_context) {
        CON_INFO("Cleaning up old WebSocket context before creating new one");
        lws_context_destroy(volc_engine->lws_context);
        volc_engine->lws_context = NULL;
        volc_engine->wsi = NULL;
    }

    if (volc_engine->state == VOLC_STATE_CONNECTED) {
        CON_INFO("WebSocket connection already active, reusing existing connection");
        return 0;
    }

    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));

    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = volc_conversation_protocols;
    info.gid = -1;
    info.uid = -1;
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.user = volc_engine;

    volc_engine->lws_context = lws_create_context(&info);
    if (!volc_engine->lws_context) {
        CON_INFO("Failed to create WebSocket context");
        return -1;
    }

    struct lws_client_connect_info ccinfo;
    memset(&ccinfo, 0, sizeof(ccinfo));

    ccinfo.context = volc_engine->lws_context;
    ccinfo.address = VOLC_HOST;
    ccinfo.port = 443;
    ccinfo.path = VOLC_PATH "?model=AG-voice-chat-agent";
    ccinfo.host = VOLC_HOST;
    ccinfo.origin = VOLC_HOST;
    ccinfo.protocol = volc_conversation_protocols[0].name;
    ccinfo.ssl_connection = \
            LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;

    volc_engine->wsi = lws_client_connect_via_info(&ccinfo);
    if (!volc_engine->wsi) {
        CON_INFO("Failed to initiate WebSocket connection");
        lws_context_destroy(volc_engine->lws_context);
        volc_engine->lws_context = NULL;
        return -1;
    }

    volc_engine->state = VOLC_STATE_CONNECTING;

    CON_INFO("WebSocket connection initiated");
    return 0;
}

static int volc_conversation_write_audio(void* engine, const char* data, int len)
{
    volc_conversation_engine_t* volc_engine = (volc_conversation_engine_t*)engine;

    if (!volc_engine || !data || len <= 0) {
        return -EINVAL;
    }

    if (volc_engine->is_finished || volc_engine->is_closed) {
        return 0;
    }

    char* audio_b64 = base64_encode((const unsigned char*)data, len);
    if (!audio_b64) {
        return -ENOMEM;
    }

    json_object* json = json_object_new_object();
    json_object_object_add(json, "type", \
                            json_object_new_string("input_audio_buffer.append"));
    json_object_object_add(json, "audio", \
                            json_object_new_string(audio_b64));

    int ret = volc_conversation_send_json_message(volc_engine, json);

    if (ret < 0) {
        return ret;
    }

    json_object_put(json);
    free(audio_b64);

    return ret;
}

static json_object *mcp_params_to_json(const mcp_param_def_t *params,
                                       size_t count)
{
    json_object *params_json = json_object_new_object();
    json_object *properties = json_object_new_object();
    json_object *required = json_object_new_array();

    for (size_t i = 0; i < count; ++i) {
        const mcp_param_def_t *param = &params[i];
        json_object *prop = json_object_new_object();
        json_object_object_add(prop, "type", json_object_new_string(param->type));
        if (param->description) {
            json_object_object_add(prop, "description", 
                                json_object_new_string(param->description));
        }
        json_object_object_add(properties, param->name, prop);
        if (param->required) {
            json_object_array_add(required, json_object_new_string(param->name));
        }
    }

    json_object_object_add(params_json, "type", json_object_new_string("object"));
    json_object_object_add(params_json, "properties", properties);
    if (json_object_array_length(required) > 0) {
        json_object_object_add(params_json, "required", required);
    } else {
        json_object_put(required);
    }
    return params_json;
}

static json_object *mcp_tool_to_json(const mcp_tool_def_t *tool)
{
    json_object *tool_json = json_object_new_object();
    json_object_object_add(tool_json, "type", json_object_new_string("function"));
    json_object_object_add(tool_json, "name", json_object_new_string(tool->name));
    if (tool->description) {
        json_object_object_add(tool_json, "description", 
                            json_object_new_string(tool->description));
    }
    if (tool->parameters && tool->param_count > 0) {
        json_object_object_add(tool_json, "parameters", 
                            mcp_params_to_json(tool->parameters, tool->param_count));
    }
    return tool_json;
}

json_object *mcp_get_tool_list(void)
{
    json_object* tool_list = json_object_new_array();
    for (size_t i = 0; i < (sizeof(g_mcp_tools) / sizeof((g_mcp_tools)[0])); ++i) {
        json_object *tool_json = mcp_tool_to_json(&g_mcp_tools[i]);
        json_object_array_add(tool_list, tool_json);
    }
    return tool_list;
}

static int volc_conversation_update_session(void* engine)
{
    volc_conversation_engine_t* volc_engine = (volc_conversation_engine_t*)engine;

    if (!volc_engine)
    {
        return -EINVAL;
    }

    json_object* json = json_object_new_object();
    json_object_object_add(json, "type", json_object_new_string("session.update"));

    json_object* session = json_object_new_object();
    json_object* session_modalities = json_object_new_array();
    json_object_array_add(session_modalities, json_object_new_string("text"));
    json_object_array_add(session_modalities, json_object_new_string("audio"));
    json_object_object_add(session, "modalities", session_modalities);
    json_object_object_add(session, "instructions",\
        json_object_new_string( "你的名字叫小v，你是一个智能助手，你的回答要尽量简短。一旦你判断字数超过200个字，\
                                 你必须精简整个回答,然后引导用户调用工具"));
    json_object_object_add(session, "voice",\
                            json_object_new_string("zh_female_tianmeiyueyue_moon_bigtts"));
    json_object_object_add(session, "input_audio_format", \
                            json_object_new_string("pcm16"));
    json_object_object_add(session, "output_audio_format", \
                json_object_new_string("pcm16"));
    json_object_object_add(session, "tool_choice", \
                            json_object_new_string("auto"));
    json_object_object_add(session, "turn_detection", NULL);

    json_object* tools_array = mcp_get_tool_list();

    json_object_object_add(session, "tools", tools_array);
    json_object_object_add(json, "session", session);

    int ret = volc_conversation_send_json_message(volc_engine, json);

    if (ret < 0) {
        return ret;
    }

    json_object_put(json);

    return ret;
}

static int volc_conversation_finish(void* engine)
{
    volc_conversation_engine_t* volc_engine = (volc_conversation_engine_t*)engine;

    if (!volc_engine) {
        return -EINVAL;
    }

    volc_engine->is_finished = true;

    // 发送第一个消息：input_audio_buffer.commit
    json_object* commit_json = json_object_new_object();
    json_object_object_add(commit_json, "type",\
                     json_object_new_string("input_audio_buffer.commit"));

    int ret = volc_conversation_send_json_message(volc_engine, commit_json);
    json_object_put(commit_json);

    if (ret < 0) {
        return ret;
    }
    json_object* json_response = json_object_new_object();
    json_object_object_add(json_response, "type", json_object_new_string("response.create"));
    json_object* response_json = json_object_new_object();
    json_object* modalities_json = json_object_new_array();
    json_object_array_add(modalities_json, json_object_new_string("text"));
    json_object_array_add(modalities_json, json_object_new_string("audio"));
    json_object_object_add(response_json, "modalities", modalities_json);
    json_object_object_add(json_response, "response", response_json);

    ret = volc_conversation_send_json_message(engine, json_response);

    volc_engine->last_response_time = lws_now_usecs();
    volc_engine->response_pending = true;

    if (ret < 0) {
        CON_ERR("Failed to send response.create to Volc");
        return ret;
    }

    json_object_put(json_response);
    return ret;
}

static int volc_conversation_cancel(void* engine)
{
    volc_conversation_engine_t* volc_engine = (volc_conversation_engine_t*)engine;

    if (!volc_engine) {
        return -EINVAL;
    }

    volc_engine->is_finished = true;

    json_object* json = json_object_new_object();
    json_object_object_add(json, "type", json_object_new_string("response.cancel"));

    int ret = volc_conversation_send_json_message(volc_engine, json);

    if (ret < 0) {
        return ret;
    }

    json_object_put(json);

    CON_INFO("Cancel: sent response.cancel, waiting for server response.done with cancelled status");

    return ret;
}

static conversation_engine_env_params_t* volc_conversation_get_env(void* engine)
{
    volc_conversation_engine_t* volc_engine = (volc_conversation_engine_t*)engine;

    if (!volc_engine) {
        return NULL;
    }

    return &volc_engine->env;
}

static int volc_conversation_send_ping(volc_conversation_engine_t *engine)
{
    if (!engine || !engine->wsi) {
        return -EINVAL;
    }

    const char* ping_data = "ping";
    size_t ping_len = strlen(ping_data);

    unsigned char* buf = malloc(ping_len + LWS_PRE);
    if (!buf) {
        return -ENOMEM;
    }

    memcpy(buf + LWS_PRE, ping_data, ping_len);

    int ret = lws_write(engine->wsi, buf + LWS_PRE,\
                     ping_len, LWS_WRITE_PING);

    free(buf);

    if (ret < 0) {
        return ret;
    }

    engine->last_ping_time = lws_now_usecs();
    engine->ping_pending = true;

    CON_INFO("Sent ping, waiting for pong");

    return ret;
}

/****************************************************************************
 * Thread Management
 ****************************************************************************/

static void* volc_conversation_loop_thread(void* arg)
{
    volc_conversation_engine_t* engine = (volc_conversation_engine_t*)arg;
    int ret;

    while (!engine->is_closed) {

        if (engine->lws_context) {
            ret = lws_service(engine->lws_context, -1);
            if (ret < 0) {
                CON_INFO("conversation lws_service failed: %d", ret);
                volc_conversation_send_event(engine, conversation_engine_event_error,
                                           "WebSocket service error", 23,
                                           conversation_engine_error_network);
                break;
            }

            if (!engine->ping_pending && \
                lws_now_usecs() - engine->last_ping_time > VOLC_PING_INTERVAL &&\
                engine->state == VOLC_STATE_CONNECTED) {
                ret = volc_conversation_send_ping(engine);
                if (ret < 0) {
                    CON_INFO("Failed to send ping: %d", ret);
                    break;
                }
            }

            if (engine->response_pending && \
                lws_now_usecs() - engine->last_response_time > VOLC_RESPONSE_TIMEOUT) {
                engine->response_pending = false;
                volc_conversation_send_event(engine, conversation_engine_event_error,
                                           "Response timeout", 13,
                                           conversation_engine_error_network);
                if (ret < 0) {
                    CON_INFO("Failed to send finish: %d", ret);
                    break;
                }
            }

        } else if (engine->is_closed && engine->lws_context) {
            // Cleanup when connection is explicitly closed
            lws_context_destroy(engine->lws_context);
            engine->lws_context = NULL;
            engine->wsi = NULL;
            CON_INFO("conversation service stopped");
            break;
        }

        if (!engine->is_running) {
            sem_post(&engine->sem);
            engine->is_running = true;
        }

        usleep(VOLC_LOOP_INTERVAL);
    }

    sem_post(&engine->sem);

    // Cleanup
    if (engine->lws_context) {
        lws_context_destroy(engine->lws_context);
        engine->lws_context = NULL;
        engine->wsi = NULL;
    }

    return NULL;
}

static int volc_conversation_create_thread(volc_conversation_engine_t* engine)
{
    struct sched_param param;
    pthread_attr_t attr;
    int ret;

    CON_INFO("Creating conversation thread");

    ret = sem_init(&engine->sem, 0, 0);
    if (ret < 0) {
        CON_ERR("Failed to init semaphore");
        return ret;
    }

    ret = pthread_rwlock_init(&engine->response_lock, NULL);

    if (ret < 0) {
        CON_ERR("Failed to init repsonse semaphore");
        return ret;
    }

    engine->is_created = false;
    engine->is_closed = false;
    engine->is_running = false;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 16384);
    param.sched_priority = 110;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&engine->thread, &attr, \
                    volc_conversation_loop_thread, engine);
    if (ret != 0) {
        CON_INFO("pthread_create failed");
        sem_destroy(&engine->sem);
        return ret;
    }

    pthread_setname_np(engine->thread, "ai_conv_volc");
    pthread_attr_destroy(&attr);

    // Wait for thread to start
    sem_wait(&engine->sem);

    CON_INFO("Conversation thread created successfully");
    return 0;
}

static int volc_conversation_destroy_thread(volc_conversation_engine_t* engine)
{
    CON_INFO("Destroying conversation thread");

    engine->is_closed = true;

    // Wait for thread to finish
    if (engine->is_running) {
        sem_wait(&engine->sem);
    }

    sem_destroy(&engine->sem);
    pthread_rwlock_destroy(&engine->response_lock);
    CON_INFO("Conversation thread destroyed");
    return 0;
}

static int volc_conversation_mcp_response(void* engine, char* result)
{
    volc_conversation_engine_t* volc_engine = (volc_conversation_engine_t*)engine;

    if (!volc_engine || !result) {
        return -1;
    }

    json_object* json = json_object_new_object();
    json_object_object_add(json, "type", \
                        json_object_new_string("conversation.item.create"));
    json_object* item = json_object_new_object();
    json_object_object_add(item, "call_id", \
                        json_object_new_string(volc_engine->call_tool_id));
    json_object_object_add(item, "type", \
                        json_object_new_string("function_call_output"));
    char result_json[256] = {0};
    snprintf(result_json, sizeof(result_json), "{\"result\":\"%s\"}", result);
    json_object_object_add(item, "output", \
                        json_object_new_string(result_json));
    json_object_object_add(json, "item", item);

    int ret = volc_conversation_send_json_message(volc_engine, json);

    if (ret < 0){
        return ret;
    }

    free(result);

    return 0;
}

/****************************************************************************
 * Plugin Definition
 ****************************************************************************/

conversation_engine_plugin_t volc_conversation_engine_plugin = {
    .name = "volc_conversation",
    .priv_size = sizeof(volc_conversation_engine_t),
    .init = volc_conversation_init,
    .uninit = volc_conversation_uninit,
    .event_cb = volc_conversation_event_cb,
    .start = volc_conversation_start,
    .write_audio = volc_conversation_write_audio,
    .finish = volc_conversation_finish,
    .cancel = volc_conversation_cancel,
    .mcp_response = volc_conversation_mcp_response,
    .get_env = volc_conversation_get_env,
};
