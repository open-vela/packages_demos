/****************************************************************************
 * src/conversation/mcp/core/streaming/mcp_server_streaming.c
 * MCP Server with Streaming/SSE Support Implementation (NuttX)
 ****************************************************************************/

#include "mcp_server_streaming.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef CONFIG_HAVE_GETTIMEOFDAY
#include <sys/time.h>
#else
#include <nuttx/clock.h>
#endif

#include <nuttx/pthread.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint64_t get_time_us(void)
{
#ifdef CONFIG_HAVE_GETTIMEOFDAY
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
#endif
}

static char *strdup_safe(const char *str)
{
    if (!str) return NULL;
    size_t len = strlen(str);
    char *dup = malloc(len + 1);
    if (dup) {
        memcpy(dup, str, len + 1);
    }
    return dup;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int mcp_streaming_server_init(mcp_streaming_server_t *server)
{
    if (!server) {
        return -EINVAL;
    }

    memset(server, 0, sizeof(mcp_streaming_server_t));
    
    int ret = pthread_rwlock_init(&server->lock, NULL);
    if (ret != 0) {
        fprintf(stderr, "Failed to initialize rwlock: %d\n", ret);
        return -ret;
    }

    server->initialized = true;
    return 0;
}

void mcp_streaming_server_destroy(mcp_streaming_server_t *server)
{
    if (!server || !server->initialized) {
        return;
    }

    pthread_rwlock_wrlock(&server->lock);
    
    server->tool_count = 0;
    server->initialized = false;
    
    pthread_rwlock_unlock(&server->lock);
    pthread_rwlock_destroy(&server->lock);
}

int mcp_streaming_server_register_tool(mcp_streaming_server_t *server,
                                       const char *name,
                                       const char *description,
                                       mcp_param_t *params,
                                       int param_count,
                                       mcp_streaming_tool_callback_t callback,
                                       void *user_data,
                                       bool is_streaming)
{
    if (!server || !server->initialized || !name || !callback) {
        return -EINVAL;
    }

    if (param_count > MCP_MAX_PARAMS) {
        fprintf(stderr, "Too many parameters: %d (max %d)\n", 
                param_count, MCP_MAX_PARAMS);
        return -EINVAL;
    }

    pthread_rwlock_wrlock(&server->lock);

    /* Check for duplicate */
    for (int i = 0; i < server->tool_count; i++) {
        if (strcmp(server->tools[i].name, name) == 0) {
            pthread_rwlock_unlock(&server->lock);
            fprintf(stderr, "Tool '%s' already registered\n", name);
            return -EEXIST;
        }
    }

    /* Check capacity */
    if (server->tool_count >= MCP_MAX_TOOLS) {
        pthread_rwlock_unlock(&server->lock);
        fprintf(stderr, "Tool registry full (%d tools)\n", MCP_MAX_TOOLS);
        return -ENOMEM;
    }

    /* Register new tool */
    mcp_streaming_tool_t *tool = &server->tools[server->tool_count];
    memset(tool, 0, sizeof(mcp_streaming_tool_t));
    
    strncpy(tool->name, name, MCP_MAX_TOOL_NAME_LEN - 1);
    if (description) {
        strncpy(tool->description, description, MCP_MAX_TOOL_DESC_LEN - 1);
    }
    
    tool->callback = callback;
    tool->user_data = user_data;
    tool->param_count = param_count;
    tool->is_streaming = is_streaming;
    
    if (params && param_count > 0) {
        memcpy(tool->params, params, sizeof(mcp_param_t) * param_count);
    }

    server->tool_count++;
    
    pthread_rwlock_unlock(&server->lock);
    
    printf("[MCP-Stream] Registered %s tool: %s (%d params)\n", 
           is_streaming ? "streaming" : "standard", name, param_count);
    return 0;
}

int mcp_streaming_server_unregister_tool(mcp_streaming_server_t *server,
                                         const char *name)
{
    if (!server || !server->initialized || !name) {
        return -EINVAL;
    }

    pthread_rwlock_wrlock(&server->lock);

    int found_idx = -1;
    for (int i = 0; i < server->tool_count; i++) {
        if (strcmp(server->tools[i].name, name) == 0) {
            found_idx = i;
            break;
        }
    }

    if (found_idx < 0) {
        pthread_rwlock_unlock(&server->lock);
        return -ENOENT;
    }

    /* Shift remaining tools */
    for (int i = found_idx; i < server->tool_count - 1; i++) {
        server->tools[i] = server->tools[i + 1];
    }
    server->tool_count--;

    pthread_rwlock_unlock(&server->lock);
    
    printf("[MCP-Stream] Unregistered tool: %s\n", name);
    return 0;
}

int mcp_streaming_server_call_tool(mcp_streaming_server_t *server,
                                   const char *tool_name,
                                   const char *params_json,
                                   mcp_stream_callback_t stream_cb,
                                   void *stream_user_data)
{
    if (!server || !server->initialized || !tool_name) {
        return -EINVAL;
    }

    pthread_rwlock_rdlock(&server->lock);

    /* Find tool */
    mcp_streaming_tool_t *tool = NULL;
    for (int i = 0; i < server->tool_count; i++) {
        if (strcmp(server->tools[i].name, tool_name) == 0) {
            tool = &server->tools[i];
            break;
        }
    }

    if (!tool) {
        pthread_rwlock_unlock(&server->lock);
        
        /* Send error through stream callback */
        if (stream_cb) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), 
                    "{\"error\":\"Tool '%s' not found\"}", tool_name);
            mcp_stream_send_error(stream_cb, stream_user_data, error_msg, 0);
        }
        return -ENOENT;
    }

    /* Execute tool */
    uint64_t start_time = get_time_us();
    
    int ret = tool->callback(params_json ? params_json : "{}", 
                            stream_cb, 
                            stream_user_data,
                            tool->user_data);
    
    uint64_t exec_time = get_time_us() - start_time;

    /* Update statistics - upgrade to write lock */
    pthread_rwlock_unlock(&server->lock);
    pthread_rwlock_wrlock(&server->lock);

    tool->call_count++;
    tool->total_exec_time_us += exec_time;
    if (ret == 0) {
        tool->success_count++;
    } else {
        tool->error_count++;
    }

    pthread_rwlock_unlock(&server->lock);

    printf("[MCP-Stream] Called %s tool '%s': code=%d, time=%llu us\n", 
           tool->is_streaming ? "streaming" : "standard",
           tool_name, ret, (unsigned long long)exec_time);

    return ret;
}

char *mcp_streaming_server_list_tools_json(mcp_streaming_server_t *server)
{
    if (!server || !server->initialized) {
        return strdup_safe("{\"error\":\"Server not initialized\"}");
    }

    pthread_rwlock_rdlock(&server->lock);

    /* Calculate required buffer size */
    size_t buf_size = 1024 + server->tool_count * 512;
    char *json = malloc(buf_size);
    if (!json) {
        pthread_rwlock_unlock(&server->lock);
        return NULL;
    }

    char *ptr = json;
    size_t remaining = buf_size;
    
    int n = snprintf(ptr, remaining, "{\"tools\":[");
    ptr += n; remaining -= n;

    for (int i = 0; i < server->tool_count; i++) {
        mcp_streaming_tool_t *tool = &server->tools[i];
        
        n = snprintf(ptr, remaining, 
                    "%s{\"name\":\"%s\",\"description\":\"%s\",\"streaming\":%s,\"parameters\":[",
                    i > 0 ? "," : "", tool->name, tool->description,
                    tool->is_streaming ? "true" : "false");
        ptr += n; remaining -= n;

        for (int j = 0; j < tool->param_count; j++) {
            mcp_param_t *param = &tool->params[j];
            const char *type_str = "string";
            switch (param->type) {
                case MCP_PARAM_TYPE_INT: type_str = "integer"; break;
                case MCP_PARAM_TYPE_FLOAT: type_str = "number"; break;
                case MCP_PARAM_TYPE_BOOL: type_str = "boolean"; break;
                case MCP_PARAM_TYPE_OBJECT: type_str = "object"; break;
                case MCP_PARAM_TYPE_ARRAY: type_str = "array"; break;
                default: type_str = "string"; break;
            }
            
            n = snprintf(ptr, remaining,
                        "%s{\"name\":\"%s\",\"type\":\"%s\",\"description\":\"%s\",\"required\":%s}",
                        j > 0 ? "," : "", param->name, type_str, param->description,
                        param->required ? "true" : "false");
            ptr += n; remaining -= n;
        }

        n = snprintf(ptr, remaining, "]}");
        ptr += n; remaining -= n;
    }

    n = snprintf(ptr, remaining, "]}");
    
    pthread_rwlock_unlock(&server->lock);
    return json;
}

/****************************************************************************
 * Streaming Utility Functions
 ****************************************************************************/

mcp_stream_chunk_t mcp_stream_chunk_create(mcp_stream_chunk_type_t type,
                                           const char *data,
                                           int sequence)
{
    mcp_stream_chunk_t chunk = {0};
    chunk.type = type;
    chunk.sequence = sequence;
    
    if (data) {
        chunk.data = strdup_safe(data);
        chunk.data_len = strlen(data);
    }
    
    return chunk;
}

void mcp_stream_chunk_free(mcp_stream_chunk_t *chunk)
{
    if (!chunk) return;
    
    if (chunk->data) {
        free(chunk->data);
        chunk->data = NULL;
    }
    chunk->data_len = 0;
}

void mcp_stream_send_data(mcp_stream_callback_t cb, void *user_data,
                         const char *data, int sequence)
{
    if (!cb || !data) return;
    
    mcp_stream_chunk_t chunk = mcp_stream_chunk_create(MCP_STREAM_CHUNK_DATA,
                                                       data, sequence);
    cb(&chunk, user_data);
    mcp_stream_chunk_free(&chunk);
}

void mcp_stream_send_error(mcp_stream_callback_t cb, void *user_data,
                          const char *error, int sequence)
{
    if (!cb || !error) return;
    
    mcp_stream_chunk_t chunk = mcp_stream_chunk_create(MCP_STREAM_CHUNK_ERROR,
                                                       error, sequence);
    cb(&chunk, user_data);
    mcp_stream_chunk_free(&chunk);
}

void mcp_stream_send_done(mcp_stream_callback_t cb, void *user_data,
                         int sequence)
{
    if (!cb) return;
    
    mcp_stream_chunk_t chunk = mcp_stream_chunk_create(MCP_STREAM_CHUNK_DONE,
                                                       "{\"status\":\"completed\"}",
                                                       sequence);
    cb(&chunk, user_data);
    mcp_stream_chunk_free(&chunk);
}
