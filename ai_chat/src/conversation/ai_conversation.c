/****************************************************************************
 * packages/demos/ai_chat/src/conversation/ai_conversation.c
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
#include <media_api.h>
#include <nuttx/pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <uv.h>
#include <uv_async_queue.h>
#include <kvdb.h>

#include "ai_log.h"
#include "ai_circular_buffer.h"
#include "ai_conversation.h"
#include "ai_conversation_plugin.h"
#include "plugin/ai_conversation_defs.h"

#define CONVERSATION_DEFAULT_TIMEOUT 30000
#define CONVERSATION_MIN_TIMEOUT 5000
#define CONVERSATION_MAX_TIMEOUT 120000
#define CONVERSATION_BUFFER_MAX_SIZE 128 * 1024

#define PLAYER_TIMER_INTERVAL_MS  500
#define PLAYER_IDLE_THRESHOLD     4    /* 4 * 500ms = 2s continuous idle */

/****************************************************************************
 * Private Types
 ****************************************************************************/

extern conversation_engine_plugin_t volc_conversation_engine_plugin;

//TBD:这个地方需要优化
typedef enum {
    CONVERSATION_STATE_INIT,
    CONVERSATION_STATE_START,
    CONVERSATION_STATE_FINISH,
    CONVERSATION_STATE_CLOSE
} conversation_state_t;

typedef struct mcp_server_s {
    uv_loop_t* loop;
    uv_async_queue_t mcp_call_asyncq;
    pthread_t mcp_server_thread;
    mcp_call_data_t mcp_call_data;// tool_name|call_id|arguments|
} mcp_server_t;

typedef struct {
    const char* name;
    const char* url;
} music_mapping_t;

static const music_mapping_t g_music_mapping[] = {
    {"k歌之王",
      CONFIG_AI_CONVERSATION_MUSIC_PATH "/Kgezhiwang.mp3"},
    {"稻香", 
      CONFIG_AI_CONVERSATION_MUSIC_PATH "/daoxiang.mp3"},
    {"同桌的你",
      CONFIG_AI_CONVERSATION_MUSIC_PATH "/tongzhuodeni.mp3"},
    {"晴天",
      CONFIG_AI_CONVERSATION_MUSIC_PATH "/qingtian.mp3"},
    {"青花瓷",
      CONFIG_AI_CONVERSATION_MUSIC_PATH "/qinghuaci.mp3"},
    {NULL,NULL}
};

typedef struct {
    const char* name;
    const char* url;
} launcher_app_mapping_t;;

static const launcher_app_mapping_t g_launcher_app_mapping[] = {
    {"会议纪要", "com.openvela.meeting"},
    {"翻译", "com.system.translation"},
    {"情感陪伴", "com.openvela.realtime.chat"},
    {"新能力发布", "com.test.asr"},
    {"塔罗牌", "com.vela.xmsdemo.tarot"},
    {"敲木鱼", "com.vela.xmsdemo.woodenfish"},
    {"电子宠物", "com.vela.xmsdemo.pet"},
    {"日历", "com.application.x4b.calendar"},
    {"设置", "com.application.x4b.settings"},
    {"白噪音", "com.application.x4b.whitenoise"},
    {"计算器", "com.vela.xmsdemo.calculator"},
    {"亲戚计算器", "com.vela.xmsdemo.relation_calculator"},
    {"录音", "com.vela.system.meeting"},
    {"情感聊天", "com.openvela.realtime.chat"},
    {"音乐播放器", "com.vela.xmsdemo.music_player"},
    {"打地鼠", "com.vela.xmsdemo.whackmole"},
    {NULL,NULL}
};

typedef struct conversation_context {
    conversation_engine_plugin_t* plugin;
    void* engine;
    void* recorder_handle; // recorder handle
    void* player_handle;   // player handle
    void* focus_handle;
    uv_loop_t* loop;
    uv_async_queue_t* asyncq;
    uv_async_queue_t user_asyncq;
    uv_pipe_t* recorder_pipe;
    uv_pipe_t* player_pipe;
    char* format;
    conversation_callback_t cb;
    void* cookie;
    conversation_state_t state;
    int is_closed;
    conversation_engine_init_params_t voice_param;
    ai_circular_buffer_t buffer;
    char* frame_buf;
    uv_write_t write_req;
    conversation_player_status_t player_status;
    mcp_server_t mcp_server;
    sem_t media_lock;
    uv_timer_t player_timer;
    int idle_count;           /* consecutive idle checks */
    bool audio_started;       /* received audio_start this round */
    bool engine_complete;     /* received engine complete this round */
    bool timer_active;        /* timer is running */
} conversation_context_t;

typedef enum {
    CONVERSATION_MESSAGE_CREATE_ENGINE,
    CONVERSATION_MESSAGE_LISTENER,
    CONVERSATION_MESSAGE_START,
    CONVERSATION_MESSAGE_FINISH,
    CONVERSATION_MESSAGE_CANCEL,
    CONVERSATION_MESSAGE_IS_BUSY,
    CONVERSATION_MESSAGE_CLOSE,
    CONVERSATION_MESSAGE_CB
} message_id_t;

typedef int (*message_handler_t)(void* message_data);

typedef struct message_s {
    message_id_t message_id;
    message_handler_t message_handler;
    void* message_data;
} message_t;

typedef struct message_data_listener_s {
    conversation_context_t* ctx;
    conversation_callback_t cb;
    void* cookie;
} message_data_listener_t;

typedef struct message_data_start_s {
    conversation_context_t* ctx;
    //TBD：拓展结构体加上audio_info_t
} message_data_start_t;

typedef struct message_data_finish_s {
    conversation_context_t* ctx;
} message_data_finish_t;

typedef struct message_data_cancel_s {
    conversation_context_t* ctx;
} message_data_cancel_t;

typedef struct message_data_close_s {
    conversation_context_t* ctx;
} message_data_close_t;

typedef struct message_data_cb_s {
    conversation_context_t* ctx;
    conversation_event_t event;
    conversation_result_t result;
} message_data_cb_t;

/****************************************************************************
 * Private Functions
 ****************************************************************************/
//Plugin Selection
static conversation_engine_plugin_t* conversation_get_plugin(conversation_engine_type engine_type);
//Async Queue Callbacks
static void conversation_async_cb(uv_async_queue_t* asyncq, void* data);
static void mcp_tool_aysync_cb(uv_async_queue_t* asyncq, void* data);
static void* mcp_tool_worker_thread(void* arg);
static int conversation_mcp_create_thread(conversation_context_t* ctx);
static int conversation_mcp_destroy_thread(conversation_context_t* ctx);
//function tools for mcp
//music tools
static const char* find_music_url(const char* music_name);
static const char* get_random_music_url(void);
static int function_call_music_play(conversation_context_t* ctx);
static int get_system_volume(void);
static int set_system_volume(int vol, int req_id);
static int function_call_adjust_volume(conversation_context_t* ctx);
static int function_call_get_volume(conversation_context_t* ctx);
static int function_call_increase_or_decrease_volume(conversation_context_t* ctx);
//launch app tools
static const char* find_app_url(const char* app_name);
static int function_call_launch_app(conversation_context_t* ctx);
static int function_call_unknown_tool(conversation_context_t* ctx);
//Engine Event Callback
static void conversation_engine_event_cb(conversation_engine_event_t event,
                                        const conversation_engine_result_t* result,
                                        void* cookie);
static void handle_mcp_call_event(conversation_context_t* ctx, \
                                const conversation_engine_result_t* result);
//Message Handlers
static int conversation_message_listener_handler(void* message_data);
static int conversation_message_start_handler(void* message_data);
static int conversation_message_finish_handler(void* message_data);
static int conversation_message_cancel_handler(void* message_data);
static int conversation_message_close_handler(void* message_data);
static void conversation_uvasyncq_close_cb(uv_handle_t* handle);
static int conversation_message_cb_handler(void* message_data);
//Media callfunctions
static int ai_conversation_init_recorder(conversation_context_t* ctx);
static int ai_conversation_init_player(conversation_context_t* ctx);
static int ai_conversation_play_audio(conversation_context_t* ctx, const void* data, int length);
//Media Callback Functions
static void alloc_read_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
static void read_buffer_cb(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);
static void media_recorder_prepare_connect_cb(void* cookie, int ret, void* obj);
static void media_recorder_open_cb(void* cookie, int ret);
static void media_recorder_start_cb(void* cookie, int ret);
static void media_recorder_pause_cb(void* cookie, int ret);
static void media_recorder_close_cb(void* cookie, int ret);
static void media_recorder_event_callback(void* cookie, int event, int ret, const char* extra);
static void media_player_prepare_connect_cb(void* cookie, int ret, void* obj);
static void media_player_open_cb(void* cookie, int ret);
static void media_player_music_start_cb(void* cookie, int ret);
static void media_player_close_cb(void* cookie, int ret);
static void media_player_stop_cb(void* cookie, int ret);
static void media_player_event_callback(void* cookie, int event, int ret, const char* extra);
static void write_audio_data_cb(uv_write_t* req, int status);
static void ai_conversation_focus_callback(int suggestion, void* cookie);
static void player_timer_cb(uv_timer_t* timer);
static int ai_conversation_map_params(conversation_context_t* ctx, \
                                     const conversation_init_params_t* in_param,
                                     conversation_engine_init_params_t* out_param);

/****************************************************************************
 * Plugin Selection
 ****************************************************************************/

static conversation_engine_plugin_t* conversation_get_plugin(conversation_engine_type engine_type)
{
    switch (engine_type) {
        case CONVERSATION_ENGINE_TYPE_VOLC:
            return &volc_conversation_engine_plugin;
        default:
            CON_ERR("Unsupported conversation engine type: %d", engine_type);
            return NULL;
    }
}

/****************************************************************************
 * Async Queue Callbacks
 ****************************************************************************/

static void conversation_async_cb(uv_async_queue_t* asyncq, void* data)
{
    message_t* message = (message_t*)data;

    if (!message || !message->message_handler) {
        CON_ERR("Invalid message in conversation async callback");
        return;
    }

    message->message_handler(message->message_data);

    if (message->message_data) {
        free(message->message_data);
    }
    free(message);
}

static const char* find_music_url(const char* music_name) {
    if (music_name == NULL || strlen(music_name) == 0) {
        return NULL;
    }

    for (int i = 0; g_music_mapping[i].name != NULL; i++) {
        if (strcmp(g_music_mapping[i].name, music_name) == 0) {
            return g_music_mapping[i].url;
        }
    }
    return NULL;
}

static const char* get_random_music_url(void) {
    int random_index = rand() % ((sizeof(g_music_mapping) / sizeof(g_music_mapping[0])) - 1);
    return g_music_mapping[random_index].url;
}

static int function_call_music_play(conversation_context_t* ctx)
{
    char* result = NULL;

    const char* music_name = ctx->mcp_server.mcp_call_data.argument;

    const char *music_url = find_music_url(music_name);

    if (strcmp(music_name, "随机音乐") == 0) {
        music_url = get_random_music_url();
    }

    if (music_url == NULL) {
        int ret = asprintf(&result, "没有找到该音乐,你可以听“稻香”、“同桌的你”、“晴天”、“青花瓷");
        if (ret < 0) {
            CON_ERR("asprintf failed");
            return -1;
        }
        ctx->plugin->mcp_response(ctx->engine, result);
        return -1;
    }

    int ret = asprintf(&result, "好的，正在播放%s", music_url ? music_name : "音乐");
    if (ret < 0) {
        CON_ERR("asprintf failed");
        return -1;
    }

    CON_INFO("music_name: %s  music_url: %s", music_name, music_url);

    ctx->plugin->mcp_response(ctx->engine, result);

    sem_wait(&ctx->media_lock);

    usleep(500 * 1000); // delay 500ms to wait for player handle

    if (!ctx->player_handle) {
        CON_ERR("mcp_tool::player_handle is NULL");
        sem_post(&ctx->media_lock);
        return -1;
    }

    CON_INFO("mcp_tool::media_uv_player_start");

    ret = media_uv_player_prepare(ctx->player_handle, \
    music_url, NULL,media_player_prepare_connect_cb, \
    NULL, NULL);
    if (ret < 0) {
        CON_ERR("mcp_tool::media_uv_player_prepare failed");
        sem_post(&ctx->media_lock);
        return -1;
    }

    ret = media_uv_player_start(ctx->player_handle, \
                    media_player_music_start_cb, ctx);
    if (ret < 0) {
        CON_ERR("mcp_tool::media_uv_player_start failed");
        sem_post(&ctx->media_lock);
        return -1;
    }

    return ret;
}

static int get_system_volume(void)
{
    // hope you to extend this function to support other volume type
    return -1;
}

static int set_system_volume(int vol, int req_id)
{
    // hope you to extend this function to support other volume type
    return -1;
}

static int function_call_get_volume(conversation_context_t* ctx)
{
    int vol = get_system_volume();
    if (vol < 0) {
        return -1;
    }
    char* result = NULL;
    int ret = asprintf(&result, "当前音量为%d", vol);
    if (ret < 0) {
        CON_ERR("asprintf failed");
        return -1;
    }
    return ctx->plugin->mcp_response(ctx->engine, result);
}

static int function_call_increase_or_decrease_volume(conversation_context_t* ctx)
{
    int vol = get_system_volume();
    char* result = NULL;
    if (vol < 0) {
        result = strdup("音量调整失败");
        ctx->plugin->mcp_response(ctx->engine, result);
        return -1;
    }
    if (strcmp(ctx->mcp_server.mcp_call_data.argument, "增加") == 0) {
        vol += 10;
    } else if (strcmp(ctx->mcp_server.mcp_call_data.argument, "减少") == 0) {
        vol -= 10;
    } else {
        CON_ERR("mcp_tool::Invalid argument: %s", ctx->mcp_server.mcp_call_data.argument);
        result = strdup("音量调整失败");
        ctx->plugin->mcp_response(ctx->engine, result);
        return -1;
    }
    if (vol > 100) {
        vol = 100;
    } else if (vol < 5) {
        vol = 5;
    }
    int ret = set_system_volume(vol, 10086); // 10086 is the temporary req_id
    if (ret < 0) {
        CON_ERR("mcp_tool::set_system_volume failed");
        result = strdup("音量调整失败");
        ctx->plugin->mcp_response(ctx->engine, result);
        return -1;
    }
    ret = asprintf(&result, "音量已调整为%d", vol);
    if (ret < 0) {
        CON_ERR("asprintf failed");
        result = strdup("音量调整失败");
        ctx->plugin->mcp_response(ctx->engine, result);
        return -1;
    }
    return ctx->plugin->mcp_response(ctx->engine, result);
}

static int function_call_adjust_volume(conversation_context_t* ctx)
{
    if (!ctx->plugin->mcp_response || !ctx->mcp_server.mcp_call_data.argument) {
        CON_ERR("mcp_tool::mcp_response or mcp_call_data is NULL");
        return -1;
    }
    int ret = 0;
    char* result = NULL;
    CON_INFO("mcp_tool_aysync_cb::adjust_volume\n");
    int volume = 0;
    char *endptr;
    long res = strtol(ctx->mcp_server.mcp_call_data.argument, &endptr, 10);
    if (*endptr != '\0') {
        CON_ERR("mcp_tool::Invalid volume value: %s", ctx->mcp_server.mcp_call_data.argument);
        result = strdup("音量调整失败");
        ctx->plugin->mcp_response(ctx->engine, result);
        return -1;
    } else {
        volume = (int)res;
    }
    CON_INFO("mcp_tool_aysync_cb::volume: %s\n", ctx->mcp_server.mcp_call_data.argument);
    if (volume > 100) {
        volume = 100;
    } else if (volume < 5) {
        volume = 5;
    }
    ret = set_system_volume(volume, 10086); // 10086 is the temporary req_id
    if (ret < 0) {
        CON_ERR("mcp_tool::set_system_volume failed");
        result = strdup("音量调整失败");
        ctx->plugin->mcp_response(ctx->engine, result);
        return ret;
    }
    result = strdup("音量已调整");

    return ctx->plugin->mcp_response(ctx->engine, result);
}

static const char* find_app_url(const char* app_name) {
    if (app_name == NULL || strlen(app_name) == 0) {
        return NULL;
    }

    for (int i = 0; g_launcher_app_mapping[i].name != NULL; i++) {
        if (strcmp(g_launcher_app_mapping[i].name, app_name) == 0) {
            return g_launcher_app_mapping[i].url;
        }
    }
    return NULL;
}

static int function_call_launch_app(conversation_context_t* ctx)
{
    if (!ctx->plugin->mcp_response || !ctx->mcp_server.mcp_call_data.argument) {
        CON_ERR("mcp_tool::mcp_response or mcp_call_data is NULL");
        return -1;
    }
    const char *app_name = ctx->mcp_server.mcp_call_data.argument;
    char* result = NULL;
    const char *app_url = find_app_url(app_name);
    if (app_url == NULL) {
        result = strdup("没有这个应用，你可以看看页面上有什么能打开的应用");
        return ctx->plugin->mcp_response(ctx->engine, result);
    }

    CON_INFO("mcp_tool_aysync_cb::launch_app\n");
    int pid = -1;


    if (pid < 0) {
        CON_INFO("launch_app::exec_builtin failed");
        result = strdup("打开应用失败");
    } else {
        CON_INFO("launch_app::exec_builtin succeed");
        result = strdup("应用已打开");
    }
    int ret = ctx->plugin->mcp_response(ctx->engine, result);
    return ret;
}

static int function_call_unknown_tool(conversation_context_t* ctx)
{
    if (!ctx->plugin->mcp_response || !ctx->mcp_server.mcp_call_data.argument) {
        CON_ERR("mcp_tool::mcp_response or mcp_call_data is NULL");
        return -1;
    }
    char* result = NULL;
    result = strdup("未知的工具");
    return ctx->plugin->mcp_response(ctx->engine, result);
}

static void mcp_tool_aysync_cb(uv_async_queue_t* asyncq, void* data)
{
    conversation_context_t* ctx = (conversation_context_t*)asyncq->data;

    char* result = NULL;

    CON_INFO("mcp_tool_aysync_cb::Enter mcp_tool_aysync_cb");

    if(!ctx|| !ctx->mcp_server.mcp_call_data.tool_name) {
        CON_ERR("mcp_tool_aysync_cb::Invalid context in conversation async callback");
        return;
    }

    if (strcmp(ctx->mcp_server.mcp_call_data.tool_name, "music_play") == 0) {
        function_call_music_play(ctx);
    } else if (strcmp(ctx->mcp_server.mcp_call_data.tool_name, "turn_light_on") == 0) {
        //only for test
        CON_INFO("mcp_tool_aysync_cb::turn_light_on");
        result = strdup("灯已打开");
        ctx->plugin->mcp_response(ctx->engine, result);
    } else if (strcmp(ctx->mcp_server.mcp_call_data.tool_name, "adjust_volume") == 0) {
        function_call_adjust_volume(ctx);
    } else if (strcmp(ctx->mcp_server.mcp_call_data.tool_name, "launch_app") == 0) {
        function_call_launch_app(ctx);
    } else if (strcmp(ctx->mcp_server.mcp_call_data.tool_name, "increase_or_decrease_volume") == 0) {
        function_call_increase_or_decrease_volume(ctx);
    } else if (strcmp(ctx->mcp_server.mcp_call_data.tool_name, "get_volume") == 0) {
        function_call_get_volume(ctx);
    } else {
        function_call_unknown_tool(ctx);
    }
}

static void* mcp_tool_worker_thread(void* arg)
{
    conversation_context_t* ctx = (conversation_context_t*)arg;

    mcp_server_t *server = &ctx->mcp_server;

    uv_loop_init(server->loop);

    ctx->mcp_server.mcp_call_asyncq.data = ctx;

    uv_async_queue_init(server->loop, &server->mcp_call_asyncq,\
                        mcp_tool_aysync_cb);

    server->loop->data = ctx;

    uv_run(server->loop, UV_RUN_DEFAULT);

    uv_loop_close(server->loop);

    return NULL;
}

static int conversation_mcp_create_thread(conversation_context_t* ctx)
{
    struct sched_param param;
    pthread_attr_t attr;
    int ret;

    CON_INFO("Creating mcp thread");

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 16384);
    param.sched_priority = 110;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&ctx->mcp_server.mcp_server_thread, &attr, \
                    mcp_tool_worker_thread, ctx);
    if (ret != 0) {
        CON_INFO("pthread_create failed");
        return ret;
    }

    pthread_setname_np(ctx->mcp_server.mcp_server_thread, "ai_mcp_server");
    pthread_attr_destroy(&attr);

    CON_INFO("Conversation thread created successfully");
    return 0;
}

static int conversation_mcp_destroy_thread(conversation_context_t* ctx)
{
    CON_INFO("Destroying mcp thread");

    uv_async_queue_close(&ctx->mcp_server.mcp_call_asyncq, NULL);
    return 0;
}

/****************************************************************************
 * Engine Event Callback
 ****************************************************************************/
static void conversation_engine_event_cb(conversation_engine_event_t event,
                                        const conversation_engine_result_t* result,
                                        void* cookie)
{
    conversation_context_t* ctx = (conversation_context_t*)cookie;

    if (!ctx) {
        CON_ERR("Invalid context in engine event callback");
        return;
    }

    //TBD: user_event 没有被使用，可以删除
    conversation_event_t user_event = conversation_event_unknown;
    switch (event) {
        case conversation_engine_event_start:
            user_event = conversation_event_start;
            CON_INFO("jiayadong::conversation_engine_event_start");
            break;
        case conversation_engine_event_stop:
            user_event = conversation_event_stop;
            CON_INFO("jiayadong::conversation_engine_event_stop");
            break;
        case conversation_engine_event_complete:
            user_event = conversation_event_complete;
            CON_INFO("jiayadong::conversation_engine_event_complete");
            break;
        case conversation_engine_event_audio:
            user_event = conversation_event_response_audio;
            break;
        case conversation_engine_event_text:
            user_event = conversation_event_response_text;
            CON_INFO("jiayadong::conversation_engine_event_text");
            break;
        case conversation_engine_event_input_text:
            user_event = conversation_event_input_text;
            break;
        case conversation_engine_event_audio_start:
            user_event = conversation_event_response_audio_start;
            break;
        case conversation_engine_event_mcp_call:
            handle_mcp_call_event(ctx, result);
            break;
        case conversation_engine_event_mcp_argument:
            user_event = conversation_event_mcp_request;
            uv_async_queue_send(&ctx->mcp_server.mcp_call_asyncq, NULL);
            break;
        case conversation_engine_event_error:
            user_event = conversation_event_error;
            break;
        case conversation_engine_event_unknown:
            user_event = conversation_event_unknown;
            break;
    }

    message_data_cb_t* cb_data = calloc(1, sizeof(message_data_cb_t));
    if (!cb_data) {
        CON_ERR("Failed to allocate callback message data");
        return;
    }

    cb_data->ctx = ctx;
    cb_data->event = user_event;

    if (result) {
        if (result->result) {
            cb_data->result.result = zalloc(result->len + 1);
            if (cb_data->result.result) {
                memcpy(cb_data->result.result, result->result, result->len);
            } else {
                free(cb_data);
                return;
            }
            cb_data->result.len = result->len;
        }

        cb_data->result.error_code = conversation_error_unknown;
        switch (result->error_code) {
            case conversation_engine_error_success:
                cb_data->result.error_code = conversation_error_success;
                break;
            case conversation_engine_error_network:
                cb_data->result.error_code = conversation_error_network;
                break;
            case conversation_engine_error_server:
                cb_data->result.error_code = conversation_error_server;
                break;
            case conversation_engine_error_cancelled:
                cb_data->result.error_code = conversation_error_cancelled;
                break;
            case conversation_engine_error_unknown:
                cb_data->result.error_code = conversation_error_unknown;
                break;
        }
    }

    message_t* message = calloc(1, sizeof(message_t));
    if (!message) {
        CON_ERR("Failed to allocate callback message");
        free(cb_data);
        return;
    }

    message->message_id = CONVERSATION_MESSAGE_CB;
    message->message_handler = conversation_message_cb_handler;
    message->message_data = cb_data;

    uv_async_queue_send(&ctx->user_asyncq, message);
}

static void handle_mcp_call_event(conversation_context_t* ctx, \
                                const conversation_engine_result_t* result)
{
    char* mcp_data = strdup((char*)result->result);
    char* saveptr = NULL;

    char* tool_name = strtok_r(mcp_data, "|", &saveptr);
    char* call_id   = strtok_r(NULL, "|", &saveptr);
    char* argument  = strtok_r(NULL, "|", &saveptr);

    if (!tool_name || !call_id || !argument) {
        free(mcp_data);
        return;
    }

    if (ctx->mcp_server.mcp_call_data.tool_name) {
        free(ctx->mcp_server.mcp_call_data.tool_name);
    }
    ctx->mcp_server.mcp_call_data.tool_name = strdup(tool_name);
    if (ctx->mcp_server.mcp_call_data.call_id) {
        free(ctx->mcp_server.mcp_call_data.call_id);
    }
    ctx->mcp_server.mcp_call_data.call_id = strdup(call_id);
    if (ctx->mcp_server.mcp_call_data.argument) {
        free(ctx->mcp_server.mcp_call_data.argument);
    }
    ctx->mcp_server.mcp_call_data.argument = strdup(argument);

    free(mcp_data);
}

/****************************************************************************
 * Message Handlers
 ****************************************************************************/

static int conversation_message_listener_handler(void* message_data)
{
    message_data_listener_t* data = (message_data_listener_t*)message_data;

    if (!data || !data->ctx) {
        return -EINVAL;
    }

    data->ctx->cb = data->cb;
    data->ctx->cookie = data->cookie;

    if (data->ctx->plugin && data->ctx->plugin->event_cb && data->ctx->engine) {
        return data->ctx->plugin->event_cb(data->ctx->engine, \
                                           conversation_engine_event_cb, data->ctx);
    }

    return 0;
}

static int conversation_message_start_handler(void* message_data)
{
    message_data_start_t* data = (message_data_start_t*)message_data;
    conversation_engine_env_params_t* env;
    int ret;

    if (!data || !data->ctx) {
        return -EINVAL;
    }

    conversation_context_t* ctx = data->ctx;

    env = ctx->plugin->get_env(ctx->engine);
    ctx->format = strdup(env->format);

    if (!ctx->recorder_handle) {
        ret = ai_conversation_init_recorder(ctx);
        if (ret < 0)
            goto failed;
    } else {
        CON_INFO("Reusing existing recorder");
    }

    ret = media_uv_recorder_start(ctx->recorder_handle, \
                        media_recorder_start_cb, ctx);
    CON_INFO("Recorder start");
    if (ret < 0)
        goto failed;

    //TBD:这里需要做修改
    if (ctx->player_status == CONVERSATION_PLAYER_STATUS_PLAYING_MUSIC) {
        ret = media_uv_player_stop(ctx->player_handle, \
                                    media_player_stop_cb, ctx);

        media_uv_player_close(ctx->player_handle, 0, \
                            media_player_close_cb);

        ctx->player_handle = media_uv_player_open(ctx->loop, "Music", \
                                                media_player_open_cb, ctx);
        ret = media_uv_player_listen(ctx->player_handle, \
                                    media_player_event_callback);
        if (ret < 0) {
            CON_ERR("conversation player listen failed");
            media_uv_player_close(ctx->player_handle, 0, \
                                media_player_close_cb);
            goto failed;
        }
    }

    ctx->audio_started = false;
    ctx->engine_complete = false;
    ctx->idle_count = 0;

    ctx->plugin->start(ctx->engine);

    ctx->state = CONVERSATION_STATE_START;

    CON_INFO("ai_conversation_start_handler");

    return 0;
failed:
    CON_ERR("ai_conversation_start_handler failed");
    if (ctx->recorder_handle) {
        media_uv_recorder_close(ctx->recorder_handle, media_recorder_close_cb);
        ctx->recorder_handle = NULL;
    }
    return ret;
}

static int conversation_message_finish_handler(void* message_data)
{
    message_data_finish_t* data = (message_data_finish_t*)message_data;

    if (!data || !data->ctx) {
        return -EINVAL;
    }

    int ret;

    conversation_context_t* ctx = data->ctx;

    if (!ctx->player_handle) {
        ret = ai_conversation_init_player(ctx);
        if (ret < 0)
            goto failed;
    } else {
        CON_INFO("Reusing existing player");
    }

    ctx->player_status = CONVERSATION_PLAYER_STATUS_PLAYING;

    CON_INFO("sem waiting ctx->media_lock");
    /*如果某轮对话只返回文本、不播语音，或播放流程在准备阶段就失败，就不会触发 media_player_stop_cb，
      导致下一次 ai_conversation_finish() 永久阻塞*/
    sem_wait(&ctx->media_lock);
    CON_INFO("sem wait done ctx->media_lock");

    ret = media_uv_recorder_pause(ctx->recorder_handle, \
                        media_recorder_pause_cb, ctx);
    if (ret < 0)
        goto failed;

    if (data->ctx->plugin && data->ctx->plugin->finish && data->ctx->engine) {
        return data->ctx->plugin->finish(data->ctx->engine);
    }

    return -ENOSYS;
failed:
    CON_ERR("ai_conversation_start_handler failed");
    if (ctx->recorder_handle) {
        media_uv_recorder_close(ctx->recorder_handle, media_recorder_close_cb);
        ctx->recorder_handle = NULL;
    }
    if (ctx->player_handle) {
        media_uv_player_close(ctx->player_handle, 0, \
                            media_player_close_cb);
        ctx->player_handle = NULL;
    }
    return ret;
}

static int conversation_message_cancel_handler(void* message_data)
{
    message_data_cancel_t* data = (message_data_cancel_t*)message_data;

    if (!data || !data->ctx) {
        return -EINVAL;
    }

    if (data->ctx->plugin && data->ctx->plugin->cancel && data->ctx->engine) {
        return data->ctx->plugin->cancel(data->ctx->engine);
    }

    return -ENOSYS;
}

static int conversation_message_close_handler(void* message_data)
{
    message_data_close_t* data = (message_data_close_t*)message_data;
    int ret = 0;

    if (!data || !data->ctx) {
        return -EINVAL;
    }

    conversation_context_t* ctx = data->ctx;

    if (ctx->state == CONVERSATION_STATE_CLOSE) {
        return 0;
    }

    ctx->is_closed = 1;
    ctx->state = CONVERSATION_STATE_CLOSE;

    uv_async_queue_close(ctx->asyncq, conversation_uvasyncq_close_cb);

    uv_async_queue_close(&ctx->user_asyncq, NULL);

    if (ctx->timer_active) {
        uv_timer_stop(&ctx->player_timer);
        ctx->timer_active = false;
    }
    uv_close((uv_handle_t*)&ctx->player_timer, NULL);

    sem_post(&ctx->media_lock);

    sem_destroy(&ctx->media_lock);

    conversation_mcp_destroy_thread(ctx);

    if (ctx->format) {
        free(ctx->format);
        ctx->format = NULL;
    }

    if (ctx->loop) {
        ctx->loop = NULL;
    }

    if (ctx->plugin && ctx->engine) {
        conversation_plugin_uninit(ctx->plugin, ctx->engine, 1);
        ctx->engine = NULL;
    }

    if (ctx->recorder_handle) {
        ret = media_uv_recorder_close(ctx->recorder_handle, \
                                    media_recorder_close_cb);
        ctx->recorder_handle = NULL;
    }

    if (ctx->player_handle) {
        ret = media_uv_player_close(ctx->player_handle, 0, \
                                    media_player_close_cb);
        ctx->player_handle = NULL;
    }

    if (ctx->player_pipe) {
        ctx->player_pipe = NULL;
    }

    if (ctx->recorder_pipe) {
        ctx->recorder_pipe = NULL;
    }

    if (ctx->focus_handle) {
        media_focus_abandon(ctx->focus_handle);
        ctx->focus_handle = NULL;
    }

    if (ctx->mcp_server.mcp_call_data.tool_name) {
        free(ctx->mcp_server.mcp_call_data.tool_name);
        ctx->mcp_server.mcp_call_data.tool_name = NULL;
    }

    if (ctx->mcp_server.mcp_call_data.call_id) {
        free(ctx->mcp_server.mcp_call_data.call_id);
        ctx->mcp_server.mcp_call_data.call_id = NULL;
    }

    if (ctx->mcp_server.mcp_call_data.argument) {
        free(ctx->mcp_server.mcp_call_data.argument);
        ctx->mcp_server.mcp_call_data.argument = NULL;
    }

    if (ctx->buffer.buffer) {
        free(ctx->buffer.buffer);
        ctx->buffer.buffer = NULL;
    }

    if (ctx->frame_buf) {
        free(ctx->frame_buf);
        ctx->frame_buf = NULL;
    }

    if (ctx->mcp_server.loop) {
        free(ctx->mcp_server.loop);
        ctx->mcp_server.loop = NULL;
    }

    CON_INFO("ai_conversation_close_handler");

    return ret;
}

static void conversation_uvasyncq_close_cb(uv_handle_t* handle)
{
    uv_async_queue_t* async_queue = (uv_async_queue_t*)handle;

    conversation_context_t* ctx = (conversation_context_t*)async_queue->data;

    if (!ctx) {
        return;
    }
    usleep(500000); // wait 500ms to ensure the mcp thread exit
    free(async_queue);
    free(ctx);

    CON_INFO("conversation_uvasyncq_close_cb - resources cleaned");
}

static int conversation_message_cb_handler(void* message_data)
{
    message_data_cb_t* data = (message_data_cb_t*)message_data;

    if (!data || !data->ctx || !data->ctx->cb) {
        return -EINVAL;
    }

    conversation_context_t* ctx = data->ctx;

    if (data->event == conversation_event_response_audio_start) {
        ctx->audio_started = true;
        int ret = media_uv_player_prepare(ctx->player_handle, NULL, ctx->format,
            media_player_prepare_connect_cb, NULL, NULL);
        if (ret < 0) {
            CON_ERR("conversation player prepare failed");
            return ret;
        }
        ret = media_uv_player_start(ctx->player_handle, NULL, ctx);
        if (ret < 0) {
            CON_ERR("conversation player start failed");
            return ret;
        }
    }

    if (data->event == conversation_event_response_audio && 
        data->result.result && data->result.len > 0) {
        ai_conversation_play_audio(ctx, data->result.result, data->result.len);
    }

    if (data->event == conversation_event_complete) {
        ctx->engine_complete = true;
        CON_INFO("engine complete: audio_started=%d", ctx->audio_started);
        if (!ctx->audio_started) {
            /* pure text reply, no audio at all — release lock directly */
            CON_INFO("No audio this round, releasing media_lock");
            sem_post(&ctx->media_lock);
        } else if (!ctx->timer_active) {
            /* audio was started, begin polling for idle buffer */
            ctx->idle_count = 0;
            ctx->player_timer.data = ctx;
            uv_timer_start(&ctx->player_timer, player_timer_cb,
                           PLAYER_TIMER_INTERVAL_MS, PLAYER_TIMER_INTERVAL_MS);
            ctx->timer_active = true;
            CON_INFO("Player idle timer started");
        }
    }

    ctx->cb(data->event, &data->result, ctx->cookie);

    if (data->result.result) {
        free((void*)data->result.result);
    }

    return 0;
}

static int ai_conversation_init_recorder(conversation_context_t* ctx)
{
    const char* format = ctx->format;
    int init_suggestion;
    char* stream = CONFIG_AI_CONVERSATION_RECORDER_STREAM;
    void* handle = NULL;

    if (!ctx) {
        return -EINVAL;
    }

    ctx->focus_handle =\
        media_focus_request(&init_suggestion, MEDIA_SCENARIO_TTS,
                            ai_conversation_focus_callback, ctx);
    if (init_suggestion != MEDIA_FOCUS_PLAY && ctx->focus_handle) {
        CON_ERR("conversation recorder focus failed");
        media_focus_abandon(ctx->focus_handle);
        ctx->focus_handle = NULL;
        goto failed;
    }

    handle = media_uv_recorder_open(ctx->loop, stream, \
                            media_recorder_open_cb, ctx);
    if (handle == NULL) {
        CON_ERR("conversation recorder open failed");
        goto failed;
    }

    int ret = media_uv_recorder_listen(handle, media_recorder_event_callback);
    if (ret < 0) {
        CON_ERR("conversation recorder listen failed");
        media_uv_recorder_close(handle, media_recorder_close_cb);
        goto failed;
    }

    ret = media_uv_recorder_prepare(handle, NULL, format,
        media_recorder_prepare_connect_cb, NULL, NULL);
    if (ret < 0) {
        CON_ERR("conversation recorder prepare failed");
        media_uv_recorder_close(handle, media_recorder_close_cb);
        goto failed;
    }

    ctx->recorder_handle = handle;
    CON_INFO("ai_conversation_init_recorder %p\n", ctx->recorder_handle);

    return 0;
failed:
    return -EPERM;
}

static int ai_conversation_init_player(conversation_context_t* ctx)
{
    char* stream = "Music";
    void* handle = NULL;

    if (!ctx) {
        return -EINVAL;
    }

    sem_init(&ctx->media_lock, 0, 1);

    handle = media_uv_player_open(ctx->loop, stream, \
                                media_player_open_cb, ctx);
    if (handle == NULL) {
        CON_ERR("conversation player open failed");
        goto failed;
    }

    int ret = media_uv_player_listen(handle, media_player_event_callback);
    if (ret < 0) {
        CON_ERR("conversation player listen failed");
        media_uv_player_close(handle, 0, media_player_close_cb);
        goto failed;
    }

    ctx->player_handle = handle;

    ctx->frame_buf = malloc(10240);
    if (!ctx->frame_buf) {
        CON_ERR("Failed to allocate audio frame buffer");
        media_uv_player_close(handle, 0, media_player_close_cb);
        goto failed;
    }

    char* buffer_data = malloc(CONVERSATION_BUFFER_MAX_SIZE);
    if (!buffer_data) {
        CON_ERR("Failed to allocate audio buffer");
        free(ctx->frame_buf);
        media_uv_player_close(handle, 0, media_player_close_cb);
        goto failed;
    }

    ai_circular_buffer_init(&ctx->buffer, buffer_data, \
                            CONVERSATION_BUFFER_MAX_SIZE);

    CON_INFO("ai_conversation_init_player %p\n", ctx->player_handle);

    return 0;
failed:
    return -EPERM;
}


static int ai_conversation_play_audio(conversation_context_t* ctx, const void* data, int length)
{
    if (!ctx || !data || length <= 0) {
        return -EINVAL;
    }

    if (ai_circular_buffer_is_full(&ctx->buffer)) {
        CON_ERR("Audio buffer full, dropping data");
        return -ENOSPC;
    }

    ctx->player_status = CONVERSATION_PLAYER_STATUS_PLAYING;

    CON_INFO("ai_conversation_play_audio %p, %d", ctx->player_handle, length);

    ai_circular_buffer_queue_arr(&ctx->buffer, (const char*)data, length);

    if (!ctx->player_pipe) {
        CON_ERR("Player pipe not initialized");
        return -EINVAL;
    }

    if (ai_circular_buffer_num_items(&ctx->buffer) > 0 && !ctx->write_req.data) {
        size_t available = ai_circular_buffer_num_items(&ctx->buffer);
        size_t to_write = available > 10240 ? 10240 : available;

        ai_circular_buffer_dequeue_arr(&ctx->buffer, ctx->frame_buf, to_write);
        if (to_write & 1) to_write--; /* 保证 16-bit 对齐 */
        if (to_write == 0) {
            return 0;
        }

        uv_buf_t buf = uv_buf_init(ctx->frame_buf, to_write);
        ctx->write_req.data = ctx;
        return uv_write(&ctx->write_req, (uv_stream_t*)ctx->player_pipe,\
                        &buf, 1, write_audio_data_cb);
    }

    return 0;
}

/****************************************************************************
 * Media Callback Functions
 ****************************************************************************/

static void alloc_read_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
    if (!handle || !buf) {
        return;
    }
    buf->base = (char*)calloc(1, suggested_size);
    buf->len = suggested_size;
}

static void read_buffer_cb(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf)
{
    conversation_context_t* ctx = uv_handle_get_data((uv_handle_t*)client);

    if (ctx && ctx->plugin && ctx->plugin->write_audio && ctx->engine &&
        nread > 0 && !ctx->is_closed && ctx->state != CONVERSATION_STATE_CLOSE) {
        ctx->plugin->write_audio(ctx->engine, buf->base, nread);
    }

    if (buf->base) {
        free(buf->base);
    }
}

static void media_recorder_prepare_connect_cb(void* cookie, int ret, void* obj)
{
    conversation_context_t* ctx = cookie;

    if (!ctx || !obj) {
        return;
    }
        
    if (ret < 0) {
        CON_ERR("conversation recorder prepare connect cb error:%d\n", ret);
        return;
    }

    ctx->recorder_pipe = (uv_pipe_t*)obj;
    uv_handle_set_data((uv_handle_t*)ctx->recorder_pipe, ctx);
    uv_read_start((uv_stream_t*)ctx->recorder_pipe, alloc_read_buffer,\
                    read_buffer_cb);
}

static void media_recorder_open_cb(void* cookie, int ret)
{
    conversation_context_t* ctx = cookie;

    if (!ctx) {
        return;
    }

    if (ret < 0) {
        CON_ERR("conversation recorder open cb error:%d", ret);
    }
    CON_INFO("conversation recorder open cb:%d", ret);
}

static void media_recorder_start_cb(void* cookie, int ret)
{
    conversation_context_t* ctx = cookie;

    if (!ctx) {
        return;
    }

    if (ret < 0) {
        CON_ERR("conversation recorder start cb error:%d", ret);
    }
    CON_INFO("conversation recorder start cb:%d", ret);
}

static void media_recorder_pause_cb(void* cookie, int ret)
{
    conversation_context_t* ctx = cookie;

    if (!ctx) {
        return;
    }

    if (ret < 0)
    {
        CON_ERR("conversation recorder pause cb error:%d", ret);
    }
    CON_INFO("conversation recorder pause cb:%d", ret);
}

static void media_recorder_close_cb(void* cookie, int ret)
{
    CON_INFO("conversation recorder close cb:%d", ret);
}

static void media_recorder_event_callback(void* cookie, int event, int ret, const char* extra)
{
    conversation_context_t* ctx = cookie;

    if (!ctx) {
        return;
    }

    if (ret < 0) {
        CON_ERR("conversation recorder event error:%d", ret);
    }

    switch (event) {
    case MEDIA_EVENT_NOP:
        CON_INFO("conversation recorder evert callback: [MEDIA_EVENT_NOP]");
        break;
    case MEDIA_EVENT_PREPARED:
        CON_INFO("conversation recorder evert callback: [MEDIA_EVENT_PREPARED]");
        break;
    case MEDIA_EVENT_STARTED:
        CON_INFO("conversation recorder evert callback: [MEDIA_EVENT_STARTED]");
        break;
    case MEDIA_EVENT_PAUSED:
        CON_INFO("conversation recorder evert callback: [MEDIA_EVENT_PAUSED]");
        break;
    case MEDIA_EVENT_STOPPED:
        CON_INFO("conversation recorder evert callback: [MEDIA_EVENT_STOPPED]");
        break;
    case MEDIA_EVENT_COMPLETED:
        CON_INFO("conversation recorder evert callback: [MEDIA_EVENT_COMPLETED]");
        break;
    default:
        return;
    }

    CON_INFO("conversation recorder event callback event:%d ret:%d", event, ret);
}

static void media_player_prepare_connect_cb(void* cookie, int ret, void* obj)
{
    conversation_context_t* ctx = cookie;

    if (!ctx || !obj) {
        return;
    }

    if (ret < 0) {
        CON_ERR("conversation player prepare connect cb error:%d\n", ret);
        return;
    }
    ctx->player_pipe = (uv_pipe_t*)obj;
    uv_handle_set_data((uv_handle_t*)ctx->player_pipe, ctx);//palyer pipe
}

static void media_player_open_cb(void* cookie, int ret)
{
    conversation_context_t* ctx = cookie;

    if (!ctx) {
        return;
    }

    ctx->player_status = CONVERSATION_PLAYER_STATUS_OPENED;

    if (ret < 0) {
        CON_ERR("conversation player open cb error:%d", ret);
    }
    CON_INFO("conversation player open cb:%d", ret);
}

static void media_player_music_start_cb(void* cookie, int ret)
{
    conversation_context_t* ctx = cookie;

    if (!ctx) {
        return;
    }

    ctx->player_status = CONVERSATION_PLAYER_STATUS_PLAYING_MUSIC;

    if (ret < 0) {
        CON_ERR("conversation player music start cb error:%d", ret);
    }
    CON_INFO("conversation player music start cb:%d", ret);
}

static void media_player_close_cb(void* cookie, int ret)
{
    conversation_context_t* ctx = cookie;

    if (!ctx) {
        return;
    }

    ctx->player_status = CONVERSATION_PLAYER_STATUS_CLOSED;

    CON_INFO("conversation player close cb:%d", ret);
}

static void player_timer_cb(uv_timer_t* timer)
{
    conversation_context_t* ctx = timer->data;

    if (!ctx || ctx->is_closed) {
        uv_timer_stop(timer);
        if (ctx)
            ctx->timer_active = false;
        return;
    }

    bool buffer_idle = (ai_circular_buffer_num_items(&ctx->buffer) == 0)
                       && (ctx->write_req.data == NULL);

    if (buffer_idle) {
        ctx->idle_count++;
    } else {
        ctx->idle_count = 0;
    }

    CON_INFO("player_timer_cb: idle_count=%d buffer_items=%zu writing=%d",
             ctx->idle_count,
             ai_circular_buffer_num_items(&ctx->buffer),
             ctx->write_req.data != NULL);

    if (ctx->idle_count >= PLAYER_IDLE_THRESHOLD) {
        CON_WARN("Player idle timeout after engine complete, forcing stop");
        uv_timer_stop(timer);
        ctx->timer_active = false;
        if (ctx->player_handle &&
            ctx->player_status == CONVERSATION_PLAYER_STATUS_PLAYING) {
            ctx->player_pipe = NULL;
            ctx->write_req.data = NULL;
            media_uv_player_stop(ctx->player_handle,
                                 media_player_stop_cb, ctx);
        } else {
            /* player never started or already stopped, just release lock */
            sem_post(&ctx->media_lock);
        }
    }
}

static void media_player_stop_cb(void* cookie, int ret)
{
    conversation_context_t* ctx = cookie;

    if (!ctx) {
        return;
    }

    if (ctx->timer_active) {
        uv_timer_stop(&ctx->player_timer);
        ctx->timer_active = false;
    }

    ctx->player_status = CONVERSATION_PLAYER_STATUS_STOPPED;

    CON_INFO("sem posting ctx->media_lock");
    sem_post(&ctx->media_lock);
    CON_INFO("sem post done ctx->media_lock");

    CON_INFO("conversation player stop cb:%d", ret);
}

static void media_player_event_callback(void* cookie, int event, int ret, const char* extra)
{
    conversation_context_t* ctx = cookie;

    if (!ctx || ctx->is_closed || !ctx->player_handle) {
        return;
    }

    if (ret < 0) {
        CON_ERR("conversation player ret error:%d", ret);
    }

    switch (event) {
    case MEDIA_EVENT_NOP:
        break;
    case MEDIA_EVENT_PREPARED:
        CON_INFO("conversation player evert callback: [MEDIA_EVENT_PREPARED]");
        break;
    case MEDIA_EVENT_STARTED:
        CON_INFO("conversation player evert callback: [MEDIA_EVENT_STARTED]");
        break;
    case MEDIA_EVENT_PAUSED:
        CON_INFO("conversation player evert callback: [MEDIA_EVENT_PAUSED]");
        break;
    case MEDIA_EVENT_STOPPED:
        CON_INFO("conversation player evert callback: [MEDIA_EVENT_STOPPED]");
        ctx->player_status = CONVERSATION_PLAYER_STATUS_STOPPED;
        break;
    case MEDIA_EVENT_COMPLETED:
        if (ctx) {
            ctx->player_pipe = NULL;
            ctx->write_req.data = NULL;
        }
        CON_INFO("conversation player evert callback: [MEDIA_EVENT_COMPLETED]");
        if (ctx->player_status != CONVERSATION_PLAYER_STATUS_STOPPED) {
            ctx->player_status = CONVERSATION_PLAYER_STATUS_STOPPED;
            media_uv_player_stop(ctx->player_handle, media_player_stop_cb, ctx);
        }
        break;
    default:
        break;
    }

    CON_INFO("conversation player evert callback: [event] :%d", event);

}

static void write_audio_data_cb(uv_write_t* req, int status)
{
    conversation_context_t* ctx = req ? (conversation_context_t*)req->data : NULL;

    if (!ctx) {
        return;
    }

    if (ctx->is_closed || ctx->state == CONVERSATION_STATE_CLOSE || !ctx->player_pipe) {
        ctx->write_req.data = NULL;
        return;
    }

    if (status < 0) {
        CON_ERR("write_audio_data_cb status error:%d", status);
        ctx->write_req.data = NULL;
        return;
    }

    if (ai_circular_buffer_num_items(&ctx->buffer) > 0) {
        size_t available = ai_circular_buffer_num_items(&ctx->buffer);

        size_t to_write = available > 10240 ? 10240 : available;
        if (to_write & 1) to_write--;
        if (to_write == 0) {
            ctx->write_req.data = NULL;
            return;
        }
        ai_circular_buffer_dequeue_arr(&ctx->buffer, ctx->frame_buf, to_write);

        uv_buf_t buf = uv_buf_init(ctx->frame_buf, to_write);
        ctx->write_req.data = ctx;
        uv_write(&ctx->write_req, (uv_stream_t*)ctx->player_pipe,\
                 &buf, 1, write_audio_data_cb);
    }else{
        ctx->write_req.data = NULL;
    }
}

static void ai_conversation_focus_callback(int suggestion, void* cookie)
{
    conversation_context_t* ctx = cookie;

    if (!ctx) {
        return;
    }

    if (suggestion != MEDIA_FOCUS_PLAY) {
        ai_conversation_finish(ctx);
    }

    CON_INFO("conversation focus suggestion:%d", suggestion);
}

static int ai_conversation_map_params(conversation_context_t* ctx, \
                                     const conversation_init_params_t* in_param,
                                     conversation_engine_init_params_t* out_param)
{
    if (!ctx || !in_param || !out_param) {
        return -EINVAL;
    }

    out_param->loop = in_param->loop;
    out_param->api_key = in_param->api_key;
    out_param->auto_next_round = in_param->auto_next_round;

    out_param->cb = conversation_async_cb;
    out_param->opaque = ctx;

    if (!out_param->loop) {
        CON_ERR("UV loop is required for conversation engine");
        return -EINVAL;
    }

    return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

conversation_handle_t ai_conversation_create_engine(const conversation_init_params_t* param)
{
    conversation_context_t* ctx;
    conversation_engine_plugin_t* plugin;

    if (!param) {
        CON_ERR("Invalid parameters for conversation engine creation");
        return NULL;
    }

    plugin = conversation_get_plugin(param->engine_type);
    if (!plugin) {
        CON_ERR("Failed to get conversation plugin");
        return NULL;
    }

    ctx = calloc(1, sizeof(conversation_context_t));
    if (!ctx) {
        CON_ERR("Failed to allocate conversation context");
        return NULL;
    }

    ctx->loop = param->loop;

    ctx->asyncq = calloc(1, sizeof(uv_async_queue_t));
    if (!ctx->asyncq) {
        CON_ERR("Failed to allocate async queue");
        free(ctx);
        return NULL;
    }

    ctx->asyncq->data = ctx;
    if (uv_async_queue_init(ctx->loop, ctx->asyncq, \
                    conversation_async_cb) < 0) {
        CON_ERR("Failed to initialize async queue");
        free(ctx->asyncq);
        free(ctx);
        return NULL;
    }

    ctx->user_asyncq.data = ctx;
    if (uv_async_queue_init(ctx->loop, &ctx->user_asyncq, \
                    conversation_async_cb) < 0) {
        CON_ERR("Failed to initialize user async queue");
        uv_close((uv_handle_t*)ctx->asyncq, NULL);
        free(ctx->asyncq);
        free(ctx);
        return NULL;
    }

    if (ai_conversation_map_params(ctx, param, &ctx->voice_param) < 0) {
        CON_ERR("Failed to map conversation parameters");
        free(ctx->asyncq);
        free(ctx);
        return NULL;
    }

    ctx->mcp_server.loop = malloc(sizeof(uv_loop_t));
    if (!ctx->mcp_server.loop) {
        CON_ERR("Failed to allocate mcp loop");
        free(ctx->asyncq);
        free(ctx);
        return NULL;
    }

    ctx->plugin = plugin;
    ctx->engine = conversation_plugin_init(plugin, &ctx->voice_param);
    if (!ctx->engine) {
        CON_ERR("Failed to initialize conversation plugin");
        free(ctx->asyncq);
        free(ctx);
        return NULL;
    }

    ctx->state = CONVERSATION_STATE_INIT;
    ctx->player_status = CONVERSATION_PLAYER_STATUS_IDLE;

    uv_timer_init(ctx->loop, &ctx->player_timer);
    ctx->player_timer.data = ctx;
    ctx->timer_active = false;
    ctx->audio_started = false;
    ctx->engine_complete = false;
    ctx->idle_count = 0;

    if (ctx->plugin && ctx->plugin->start && ctx->engine) {
        ctx->plugin->start(ctx->engine);
    }

    conversation_mcp_create_thread(ctx);

    CON_INFO("Conversation engine created successfully");
    return (conversation_handle_t)ctx;
}

int ai_conversation_set_listener(conversation_handle_t handle,
                                conversation_callback_t callback,
                                void* cookie)
{
    conversation_context_t* ctx = (conversation_context_t*)handle;

    if (!ctx || !callback) {
        return -EINVAL;
    }

    if (ctx->is_closed) {
        return -EBADF;
    }

    message_data_listener_t* data = calloc(1, sizeof(message_data_listener_t));
    if (!data) {
        return -ENOMEM;
    }

    data->ctx = ctx;
    data->cb = callback;
    data->cookie = cookie;

    message_t* message = calloc(1, sizeof(message_t));
    if (!message) {
        free(data);
        return -ENOMEM;
    }

    message->message_id = CONVERSATION_MESSAGE_LISTENER;
    message->message_handler = conversation_message_listener_handler;
    message->message_data = data;

    return uv_async_queue_send(ctx->asyncq, message);
}

int ai_conversation_start(conversation_handle_t handle)
{
    conversation_context_t* ctx = (conversation_context_t*)handle;

    if (!ctx) {
        return -EINVAL;
    }

    if (ctx->is_closed) {
        return -EBADF;
    }

    message_data_start_t* data = zalloc(sizeof(message_data_start_t));
    if (!data) {
        return -ENOMEM;
    }

    data->ctx = ctx;

    message_t* message = zalloc(sizeof(message_t));
    if (!message) {
        free(data);
        return -ENOMEM;
    }

    message->message_id = CONVERSATION_MESSAGE_START;
    message->message_handler = conversation_message_start_handler;
    message->message_data = data;

    CON_INFO("conversation event = [ai_conversation_start]\n");

    return uv_async_queue_send(ctx->asyncq, message);
}

int ai_conversation_finish(conversation_handle_t handle)
{
    conversation_context_t* ctx = (conversation_context_t*)handle;

    if (!ctx) {
        return -EINVAL;
    }

    if (ctx->is_closed) {
        return -EBADF;
    }

    message_data_finish_t* data = calloc(1, sizeof(message_data_finish_t));
    if (!data) {
        return -ENOMEM;
    }

    data->ctx = ctx;

    message_t* message = calloc(1, sizeof(message_t));
    if (!message) {
        free(data);
        return -ENOMEM;
    }

    message->message_id = CONVERSATION_MESSAGE_FINISH;
    message->message_handler = conversation_message_finish_handler;
    message->message_data = data;

    CON_INFO("conversation event = [ai_conversation_finish]\n");

    return uv_async_queue_send(ctx->asyncq, message);
}

int ai_conversation_cancel(conversation_handle_t handle)
{
    conversation_context_t* ctx = (conversation_context_t*)handle;

    if (!ctx) {
        return -EINVAL;
    }

    if (ctx->is_closed) {
        return -EBADF;
    }

    message_data_cancel_t* data = calloc(1, sizeof(message_data_cancel_t));
    if (!data) {
        return -ENOMEM;
    }

    data->ctx = ctx;

    message_t* message = calloc(1, sizeof(message_t));
    if (!message) {
        free(data);
        return -ENOMEM;
    }

    message->message_id = CONVERSATION_MESSAGE_CANCEL;
    message->message_handler = conversation_message_cancel_handler;
    message->message_data = data;

    return uv_async_queue_send(ctx->asyncq, message);
}

// by player_status to judge whether the conversation is busy
int ai_conversation_is_busy(conversation_handle_t handle)
{
    conversation_context_t* ctx = (conversation_context_t*)handle;

    if (!ctx) {
        return -EINVAL;
    }

    if (ctx->is_closed) {
        return -EBADF;
    }

    return (ctx->player_status == CONVERSATION_PLAYER_STATUS_PLAYING) ? 1 : 0;
}

int ai_conversation_close(conversation_handle_t handle)
{
    conversation_context_t* ctx = (conversation_context_t*)handle;

    if (!ctx) {
        return -EINVAL;
    }

    if (ctx->is_closed) {
        return 0;
    }

    message_data_close_t* data = calloc(1, sizeof(message_data_close_t));
    if (!data) {
        return -ENOMEM;
    }

    data->ctx = ctx;

    message_t* message = calloc(1, sizeof(message_t));
    if (!message) {
        free(data);
        return -ENOMEM;
    }

    message->message_id = CONVERSATION_MESSAGE_CLOSE;
    message->message_handler = conversation_message_close_handler;
    message->message_data = data;

    return uv_async_queue_send(ctx->asyncq, message);
}
