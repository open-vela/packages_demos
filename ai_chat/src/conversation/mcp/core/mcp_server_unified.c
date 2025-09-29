/****************************************************************************
 * src/conversation/mcp/mcp_server_unified.c
 * Unified MCP Server Implementation
 ****************************************************************************/

#include "mcp_server_unified.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netutils/cJSON.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/* JSON-RPC error codes */
#define JSONRPC_PARSE_ERROR      -32700
#define JSONRPC_INVALID_REQUEST  -32600
#define JSONRPC_METHOD_NOT_FOUND -32601
#define JSONRPC_INVALID_PARAMS   -32602
#define JSONRPC_INTERNAL_ERROR   -32603

static char *create_error_response(const char *id, int code, const char *message)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "jsonrpc", "2.0");
    
    cJSON *error = cJSON_CreateObject();
    cJSON_AddNumberToObject(error, "code", code);
    cJSON_AddStringToObject(error, "message", message);
    cJSON_AddItemToObject(root, "error", error);
    
    if (id) {
        cJSON_AddStringToObject(root, "id", id);
    } else {
        cJSON_AddNullToObject(root, "id");
    }
    
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json;
}

static char *create_success_response(const char *id, cJSON *result)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "jsonrpc", "2.0");
    cJSON_AddItemToObject(root, "result", result);
    
    if (id) {
        cJSON_AddStringToObject(root, "id", id);
    } else {
        cJSON_AddNullToObject(root, "id");
    }
    
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json;
}

/* ==================== Tools Handlers ==================== */

static char *handle_tools_list(mcp_server_t *server, const char *id)
{
    pthread_rwlock_rdlock(&server->tool_lock);
    
    cJSON *tools_array = cJSON_CreateArray();
    
    for (int i = 0; i < server->tool_count; i++) {
        mcp_tool_t *tool = &server->tools[i];
        
        cJSON *tool_obj = cJSON_CreateObject();
        cJSON_AddStringToObject(tool_obj, "name", tool->name);
        cJSON_AddStringToObject(tool_obj, "description", tool->description);
        
        if (tool->param_count > 0) {
            cJSON *params = cJSON_CreateObject();
            cJSON *properties = cJSON_CreateObject();
            cJSON *required = cJSON_CreateArray();
            
            for (int j = 0; j < tool->param_count; j++) {
                cJSON *param_obj = cJSON_CreateObject();
                cJSON_AddStringToObject(param_obj, "type", "string");
                cJSON_AddStringToObject(param_obj, "description", tool->params[j].description);
                cJSON_AddItemToObject(properties, tool->params[j].name, param_obj);
                
                if (tool->params[j].required) {
                    cJSON_AddItemToArray(required, cJSON_CreateString(tool->params[j].name));
                }
            }
            
            cJSON_AddStringToObject(params, "type", "object");
            cJSON_AddItemToObject(params, "properties", properties);
            if (cJSON_GetArraySize(required) > 0) {
                cJSON_AddItemToObject(params, "required", required);
            } else {
                cJSON_Delete(required);
            }
            
            cJSON_AddItemToObject(tool_obj, "inputSchema", params);
        }
        
        cJSON_AddItemToArray(tools_array, tool_obj);
    }
    
    pthread_rwlock_unlock(&server->tool_lock);
    
    cJSON *result = cJSON_CreateObject();
    cJSON_AddItemToObject(result, "tools", tools_array);
    
    return create_success_response(id, result);
}

static char *handle_tools_call(mcp_server_t *server, const char *id, cJSON *params)
{
    cJSON *name_obj = cJSON_GetObjectItem(params, "name");
    cJSON *args_obj = cJSON_GetObjectItem(params, "arguments");
    
    if (!name_obj || !cJSON_IsString(name_obj)) {
        return create_error_response(id, JSONRPC_INVALID_PARAMS, "Missing tool name");
    }
    
    const char *tool_name = name_obj->valuestring;
    char *args_json = args_obj ? cJSON_PrintUnformatted(args_obj) : strdup("{}");
    
    pthread_rwlock_rdlock(&server->tool_lock);
    
    mcp_tool_t *tool = NULL;
    for (int i = 0; i < server->tool_count; i++) {
        if (strcmp(server->tools[i].name, tool_name) == 0) {
            tool = &server->tools[i];
            break;
        }
    }
    
    if (!tool) {
        pthread_rwlock_unlock(&server->tool_lock);
        free(args_json);
        return create_error_response(id, JSONRPC_METHOD_NOT_FOUND, "Tool not found");
    }
    
    tool->call_count++;
    char *result_str = tool->callback(args_json, tool->user_data);
    
    pthread_rwlock_unlock(&server->tool_lock);
    free(args_json);
    
    if (!result_str) {
        return create_error_response(id, JSONRPC_INTERNAL_ERROR, "Tool execution failed");
    }
    
    cJSON *result = cJSON_CreateObject();
    cJSON *content_array = cJSON_CreateArray();
    cJSON *content_obj = cJSON_CreateObject();
    cJSON_AddStringToObject(content_obj, "type", "text");
    cJSON_AddStringToObject(content_obj, "text", result_str);
    cJSON_AddItemToArray(content_array, content_obj);
    cJSON_AddItemToObject(result, "content", content_array);
    
    free(result_str);
    return create_success_response(id, result);
}

/* ==================== Resources Handlers ==================== */

static char *handle_resources_list(mcp_server_t *server, const char *id)
{
    pthread_rwlock_rdlock(&server->resource_lock);
    
    cJSON *resources_array = cJSON_CreateArray();
    
    for (int i = 0; i < server->resource_count; i++) {
        mcp_resource_t *res = &server->resources[i];
        
        cJSON *res_obj = cJSON_CreateObject();
        cJSON_AddStringToObject(res_obj, "uri", res->uri);
        cJSON_AddStringToObject(res_obj, "name", res->name);
        cJSON_AddStringToObject(res_obj, "description", res->description);
        cJSON_AddStringToObject(res_obj, "mimeType", res->mime_type);
        
        cJSON_AddItemToArray(resources_array, res_obj);
    }
    
    pthread_rwlock_unlock(&server->resource_lock);
    
    cJSON *result = cJSON_CreateObject();
    cJSON_AddItemToObject(result, "resources", resources_array);
    
    return create_success_response(id, result);
}

static char *handle_resources_read(mcp_server_t *server, const char *id, cJSON *params)
{
    cJSON *uri_obj = cJSON_GetObjectItem(params, "uri");
    
    if (!uri_obj || !cJSON_IsString(uri_obj)) {
        return create_error_response(id, JSONRPC_INVALID_PARAMS, "Missing resource URI");
    }
    
    const char *uri = uri_obj->valuestring;
    
    pthread_rwlock_rdlock(&server->resource_lock);
    
    mcp_resource_t *resource = NULL;
    for (int i = 0; i < server->resource_count; i++) {
        if (strcmp(server->resources[i].uri, uri) == 0) {
            resource = &server->resources[i];
            break;
        }
    }
    
    if (!resource) {
        pthread_rwlock_unlock(&server->resource_lock);
        return create_error_response(id, JSONRPC_METHOD_NOT_FOUND, "Resource not found");
    }
    
    char *content = resource->read_callback(uri, resource->user_data);
    
    pthread_rwlock_unlock(&server->resource_lock);
    
    if (!content) {
        return create_error_response(id, JSONRPC_INTERNAL_ERROR, "Failed to read resource");
    }
    
    cJSON *result = cJSON_CreateObject();
    cJSON *contents_array = cJSON_CreateArray();
    cJSON *content_obj = cJSON_CreateObject();
    cJSON_AddStringToObject(content_obj, "uri", uri);
    cJSON_AddStringToObject(content_obj, "mimeType", resource->mime_type);
    cJSON_AddStringToObject(content_obj, "text", content);
    cJSON_AddItemToArray(contents_array, content_obj);
    cJSON_AddItemToObject(result, "contents", contents_array);
    
    free(content);
    return create_success_response(id, result);
}

/* ==================== Prompts Handlers ==================== */

static char *handle_prompts_list(mcp_server_t *server, const char *id)
{
    pthread_rwlock_rdlock(&server->prompt_lock);
    
    cJSON *prompts_array = cJSON_CreateArray();
    
    for (int i = 0; i < server->prompt_count; i++) {
        mcp_prompt_t *prompt = &server->prompts[i];
        
        cJSON *prompt_obj = cJSON_CreateObject();
        cJSON_AddStringToObject(prompt_obj, "name", prompt->name);
        cJSON_AddStringToObject(prompt_obj, "description", prompt->description);
        
        if (prompt->arg_count > 0) {
            cJSON *args_array = cJSON_CreateArray();
            for (int j = 0; j < prompt->arg_count; j++) {
                cJSON *arg_obj = cJSON_CreateObject();
                cJSON_AddStringToObject(arg_obj, "name", prompt->args[j].name);
                cJSON_AddStringToObject(arg_obj, "description", prompt->args[j].description);
                cJSON_AddBoolToObject(arg_obj, "required", prompt->args[j].required);
                cJSON_AddItemToArray(args_array, arg_obj);
            }
            cJSON_AddItemToObject(prompt_obj, "arguments", args_array);
        }
        
        cJSON_AddItemToArray(prompts_array, prompt_obj);
    }
    
    pthread_rwlock_unlock(&server->prompt_lock);
    
    cJSON *result = cJSON_CreateObject();
    cJSON_AddItemToObject(result, "prompts", prompts_array);
    
    return create_success_response(id, result);
}

static char *handle_prompts_get(mcp_server_t *server, const char *id, cJSON *params)
{
    cJSON *name_obj = cJSON_GetObjectItem(params, "name");
    cJSON *args_obj = cJSON_GetObjectItem(params, "arguments");
    
    if (!name_obj || !cJSON_IsString(name_obj)) {
        return create_error_response(id, JSONRPC_INVALID_PARAMS, "Missing prompt name");
    }
    
    const char *prompt_name = name_obj->valuestring;
    char *args_json = args_obj ? cJSON_PrintUnformatted(args_obj) : NULL;
    
    pthread_rwlock_rdlock(&server->prompt_lock);
    
    mcp_prompt_t *prompt = NULL;
    for (int i = 0; i < server->prompt_count; i++) {
        if (strcmp(server->prompts[i].name, prompt_name) == 0) {
            prompt = &server->prompts[i];
            break;
        }
    }
    
    if (!prompt) {
        pthread_rwlock_unlock(&server->prompt_lock);
        free(args_json);
        return create_error_response(id, JSONRPC_METHOD_NOT_FOUND, "Prompt not found");
    }
    
    int message_count = 0;
    mcp_prompt_message_t *messages = prompt->callback(prompt_name, args_json,
                                                      &message_count, prompt->user_data);
    
    pthread_rwlock_unlock(&server->prompt_lock);
    free(args_json);
    
    if (!messages || message_count == 0) {
        return create_error_response(id, JSONRPC_INTERNAL_ERROR, "Failed to generate prompt");
    }
    
    cJSON *result = cJSON_CreateObject();
    cJSON *messages_array = cJSON_CreateArray();
    
    for (int i = 0; i < message_count; i++) {
        cJSON *msg_obj = cJSON_CreateObject();
        cJSON_AddStringToObject(msg_obj, "role", messages[i].role);
        
        cJSON *content_obj = cJSON_CreateObject();
        cJSON_AddStringToObject(content_obj, "type", "text");
        cJSON_AddStringToObject(content_obj, "text", messages[i].content);
        cJSON_AddItemToObject(msg_obj, "content", content_obj);
        
        cJSON_AddItemToArray(messages_array, msg_obj);
        free(messages[i].content);
    }
    free(messages);
    
    cJSON_AddItemToObject(result, "messages", messages_array);
    
    return create_success_response(id, result);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/* ==================== Server Lifecycle ==================== */

int mcp_server_init(mcp_server_t *server, const char *name, const char *version)
{
    if (!server || !name || !version) {
        return -1;
    }
    
    memset(server, 0, sizeof(*server));
    
    strncpy(server->name, name, sizeof(server->name) - 1);
    strncpy(server->version, version, sizeof(server->version) - 1);
    
    pthread_rwlock_init(&server->tool_lock, NULL);
    pthread_rwlock_init(&server->resource_lock, NULL);
    pthread_rwlock_init(&server->prompt_lock, NULL);
    
    server->initialized = true;
    return 0;
}

void mcp_server_destroy(mcp_server_t *server)
{
    if (!server || !server->initialized) {
        return;
    }
    
    if (server->stdio_running) {
        mcp_server_stop_stdio(server);
    }
    
    pthread_rwlock_destroy(&server->tool_lock);
    pthread_rwlock_destroy(&server->resource_lock);
    pthread_rwlock_destroy(&server->prompt_lock);
    
    server->initialized = false;
}

/* ==================== Tools ==================== */

int mcp_server_register_tool(mcp_server_t *server,
                              const char *name,
                              const char *description,
                              const mcp_param_t *params,
                              int param_count,
                              mcp_tool_fn callback,
                              void *user_data,
                              bool is_streaming)
{
    if (!server || !name || !callback || param_count > MCP_MAX_PARAMS) {
        return -1;
    }
    
    pthread_rwlock_wrlock(&server->tool_lock);
    
    if (server->tool_count >= MCP_MAX_TOOLS) {
        pthread_rwlock_unlock(&server->tool_lock);
        return -1;
    }
    
    mcp_tool_t *tool = &server->tools[server->tool_count];
    strncpy(tool->name, name, sizeof(tool->name) - 1);
    strncpy(tool->description, description ? description : "", sizeof(tool->description) - 1);
    
    tool->param_count = param_count;
    if (params && param_count > 0) {
        memcpy(tool->params, params, param_count * sizeof(mcp_param_t));
    }
    
    tool->callback = callback;
    tool->user_data = user_data;
    tool->is_streaming = is_streaming;
    tool->call_count = 0;
    
    server->tool_count++;
    pthread_rwlock_unlock(&server->tool_lock);
    
    return 0;
}

/* ==================== Resources ==================== */

int mcp_server_register_resource(mcp_server_t *server,
                                  const char *uri,
                                  const char *name,
                                  const char *description,
                                  const char *mime_type,
                                  mcp_resource_fn callback,
                                  void *user_data)
{
    if (!server || !uri || !name || !callback) {
        return -1;
    }
    
    pthread_rwlock_wrlock(&server->resource_lock);
    
    if (server->resource_count >= MCP_MAX_RESOURCES) {
        pthread_rwlock_unlock(&server->resource_lock);
        return -1;
    }
    
    mcp_resource_t *res = &server->resources[server->resource_count];
    strncpy(res->uri, uri, sizeof(res->uri) - 1);
    strncpy(res->name, name, sizeof(res->name) - 1);
    strncpy(res->description, description ? description : "", sizeof(res->description) - 1);
    strncpy(res->mime_type, mime_type ? mime_type : "text/plain", sizeof(res->mime_type) - 1);
    
    res->read_callback = callback;
    res->user_data = user_data;
    
    server->resource_count++;
    pthread_rwlock_unlock(&server->resource_lock);
    
    return 0;
}

/* ==================== Prompts ==================== */

int mcp_server_register_prompt(mcp_server_t *server,
                                const char *name,
                                const char *description,
                                const mcp_param_t *args,
                                int arg_count,
                                mcp_prompt_fn callback,
                                void *user_data)
{
    if (!server || !name || !callback || arg_count > MCP_MAX_PARAMS) {
        return -1;
    }
    
    pthread_rwlock_wrlock(&server->prompt_lock);
    
    if (server->prompt_count >= MCP_MAX_PROMPTS) {
        pthread_rwlock_unlock(&server->prompt_lock);
        return -1;
    }
    
    mcp_prompt_t *prompt = &server->prompts[server->prompt_count];
    strncpy(prompt->name, name, sizeof(prompt->name) - 1);
    strncpy(prompt->description, description ? description : "", sizeof(prompt->description) - 1);
    
    prompt->arg_count = arg_count;
    if (args && arg_count > 0) {
        memcpy(prompt->args, args, arg_count * sizeof(mcp_param_t));
    }
    
    prompt->callback = callback;
    prompt->user_data = user_data;
    
    server->prompt_count++;
    pthread_rwlock_unlock(&server->prompt_lock);
    
    return 0;
}

/* ==================== Protocol Handler ==================== */

char *mcp_server_handle_request(mcp_server_t *server, const char *request_json)
{
    if (!server || !request_json) {
        return create_error_response(NULL, JSONRPC_INVALID_REQUEST, "Invalid request");
    }
    
    cJSON *root = cJSON_Parse(request_json);
    if (!root) {
        return create_error_response(NULL, JSONRPC_PARSE_ERROR, "Parse error");
    }
    
    cJSON *jsonrpc = cJSON_GetObjectItem(root, "jsonrpc");
    cJSON *method = cJSON_GetObjectItem(root, "method");
    cJSON *params = cJSON_GetObjectItem(root, "params");
    cJSON *id = cJSON_GetObjectItem(root, "id");
    
    const char *id_str = NULL;
    if (id && cJSON_IsString(id)) {
        id_str = id->valuestring;
    } else if (id && cJSON_IsNumber(id)) {
        static char id_buf[32];
        snprintf(id_buf, sizeof(id_buf), "%lld", id->valueint);
        id_str = id_buf;
    }
    
    if (!jsonrpc || !method || !cJSON_IsString(method)) {
        cJSON_Delete(root);
        return create_error_response(id_str, JSONRPC_INVALID_REQUEST, "Invalid request");
    }
    
    const char *method_name = method->valuestring;
    char *response = NULL;
    
    if (strcmp(method_name, "tools/list") == 0) {
        response = handle_tools_list(server, id_str);
    } else if (strcmp(method_name, "tools/call") == 0) {
        response = handle_tools_call(server, id_str, params);
    } else if (strcmp(method_name, "resources/list") == 0) {
        response = handle_resources_list(server, id_str);
    } else if (strcmp(method_name, "resources/read") == 0) {
        response = handle_resources_read(server, id_str, params);
    } else if (strcmp(method_name, "prompts/list") == 0) {
        response = handle_prompts_list(server, id_str);
    } else if (strcmp(method_name, "prompts/get") == 0) {
        response = handle_prompts_get(server, id_str, params);
    } else {
        response = create_error_response(id_str, JSONRPC_METHOD_NOT_FOUND, "Method not found");
    }
    
    cJSON_Delete(root);
    return response;
}

/* ==================== stdio Transport ==================== */

static void *stdio_thread(void *arg)
{
    mcp_server_t *server = (mcp_server_t *)arg;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    
    fprintf(stderr, "[MCP] stdio server started\n");
    
    while (server->stdio_running) {
        read = getline(&line, &len, stdin);
        if (read <= 0) {
            break;
        }
        
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        
        fprintf(stderr, "[MCP] <- %s\n", line);
        
        char *response = mcp_server_handle_request(server, line);
        if (response) {
            fprintf(stdout, "%s\n", response);
            fflush(stdout);
            fprintf(stderr, "[MCP] -> %s\n", response);
            free(response);
        }
    }
    
    free(line);
    fprintf(stderr, "[MCP] stdio server stopped\n");
    return NULL;
}

int mcp_server_start_stdio(mcp_server_t *server)
{
    if (!server || server->stdio_running) {
        return -1;
    }
    
    server->stdio_running = true;
    
    if (pthread_create(&server->stdio_thread, NULL, stdio_thread, server) != 0) {
        server->stdio_running = false;
        return -1;
    }
    
    return 0;
}

void mcp_server_stop_stdio(mcp_server_t *server)
{
    if (!server || !server->stdio_running) {
        return;
    }
    
    server->stdio_running = false;
    pthread_cancel(server->stdio_thread);
    pthread_join(server->stdio_thread, NULL);
}
