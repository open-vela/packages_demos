/****************************************************************************
 * src/conversation/mcp/core/protocol/mcp_protocol_full.c
 * Complete MCP Protocol Implementation (NuttX)
 ****************************************************************************/

#include "mcp_protocol_full.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nuttx/pthread.h>

/* cJSON support - check if available in NuttX */
#ifdef CONFIG_EXTERNALS_CJSON
#include <cjson/cJSON.h>
#else
/* Fallback: manual JSON parsing if cJSON not available */
#warning "cJSON not available, using simplified JSON parsing"
#endif

/****************************************************************************
 * Server Lifecycle
 ****************************************************************************/

int mcp_full_server_init(mcp_full_server_t *server,
                         const char *name,
                         const char *version)
{
    if (!server || !name || !version) {
        return -1;
    }

    memset(server, 0, sizeof(*server));

    /* Initialize tool server */
    if (mcp_streaming_server_init(&server->tool_server) != 0) {
        return -1;
    }

    /* Initialize resources */
    pthread_rwlock_init(&server->resource_lock, NULL);
    server->resource_count = 0;

    /* Initialize prompts */
    pthread_rwlock_init(&server->prompt_lock, NULL);
    server->prompt_count = 0;

    /* Set server info */
    strncpy(server->server_name, name, sizeof(server->server_name) - 1);
    strncpy(server->server_version, version, sizeof(server->server_version) - 1);

    server->initialized = true;
    return 0;
}

void mcp_full_server_destroy(mcp_full_server_t *server)
{
    if (!server || !server->initialized) {
        return;
    }

    mcp_streaming_server_destroy(&server->tool_server);
    pthread_rwlock_destroy(&server->resource_lock);
    pthread_rwlock_destroy(&server->prompt_lock);

    server->initialized = false;
}

/****************************************************************************
 * Resources API
 ****************************************************************************/

int mcp_resource_register(mcp_full_server_t *server,
                         const char *uri,
                         const char *name,
                         const char *description,
                         const char *mime_type,
                         mcp_resource_type_t type,
                         char *(*read_callback)(const char *uri, void *user_data),
                         void *user_data)
{
    if (!server || !uri || !name || !read_callback) {
        return -1;
    }

    pthread_rwlock_wrlock(&server->resource_lock);

    if (server->resource_count >= MCP_MAX_RESOURCES) {
        pthread_rwlock_unlock(&server->resource_lock);
        return -1;
    }

    /* Check for duplicate URI */
    for (int i = 0; i < server->resource_count; i++) {
        if (strcmp(server->resources[i].uri, uri) == 0) {
            pthread_rwlock_unlock(&server->resource_lock);
            return -1;
        }
    }

    mcp_resource_t *res = &server->resources[server->resource_count];
    strncpy(res->uri, uri, sizeof(res->uri) - 1);
    strncpy(res->name, name, sizeof(res->name) - 1);
    strncpy(res->description, description ? description : "",
            sizeof(res->description) - 1);
    strncpy(res->mime_type, mime_type ? mime_type : "text/plain",
            sizeof(res->mime_type) - 1);
    res->type = type;
    res->read_callback = read_callback;
    res->user_data = user_data;

    server->resource_count++;
    pthread_rwlock_unlock(&server->resource_lock);

    return 0;
}

int mcp_resource_unregister(mcp_full_server_t *server, const char *uri)
{
    if (!server || !uri) {
        return -1;
    }

    pthread_rwlock_wrlock(&server->resource_lock);

    for (int i = 0; i < server->resource_count; i++) {
        if (strcmp(server->resources[i].uri, uri) == 0) {
            /* Move remaining resources down */
            memmove(&server->resources[i],
                   &server->resources[i + 1],
                   (server->resource_count - i - 1) * sizeof(mcp_resource_t));
            server->resource_count--;
            pthread_rwlock_unlock(&server->resource_lock);
            return 0;
        }
    }

    pthread_rwlock_unlock(&server->resource_lock);
    return -1;
}

char *mcp_resource_list(mcp_full_server_t *server)
{
    if (!server) {
        return NULL;
    }

    pthread_rwlock_rdlock(&server->resource_lock);

    cJSON *root = cJSON_CreateObject();
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

    cJSON_AddItemToObject(root, "resources", resources_array);
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    pthread_rwlock_unlock(&server->resource_lock);
    return json;
}

char *mcp_resource_read(mcp_full_server_t *server, const char *uri)
{
    if (!server || !uri) {
        return NULL;
    }

    pthread_rwlock_rdlock(&server->resource_lock);

    for (int i = 0; i < server->resource_count; i++) {
        if (strcmp(server->resources[i].uri, uri) == 0) {
            mcp_resource_t *res = &server->resources[i];
            char *content = res->read_callback(uri, res->user_data);
            
            pthread_rwlock_unlock(&server->resource_lock);
            
            if (!content) {
                return NULL;
            }

            /* Create JSON response */
            cJSON *root = cJSON_CreateObject();
            cJSON_AddStringToObject(root, "uri", uri);
            cJSON_AddStringToObject(root, "mimeType", res->mime_type);
            cJSON_AddStringToObject(root, "text", content);
            
            char *json = cJSON_PrintUnformatted(root);
            cJSON_Delete(root);
            free(content);
            
            return json;
        }
    }

    pthread_rwlock_unlock(&server->resource_lock);
    return NULL;
}

/****************************************************************************
 * Prompts API
 ****************************************************************************/

int mcp_prompt_register(mcp_full_server_t *server,
                       const char *name,
                       const char *description,
                       mcp_prompt_arg_t *args,
                       int arg_count,
                       mcp_prompt_message_t *(*get_callback)(const char *name,
                                                            const char *args_json,
                                                            int *message_count,
                                                            void *user_data),
                       void *user_data)
{
    if (!server || !name || !get_callback) {
        return -1;
    }

    if (arg_count > MCP_MAX_PARAMS) {
        return -1;
    }

    pthread_rwlock_wrlock(&server->prompt_lock);

    if (server->prompt_count >= MCP_MAX_PROMPTS) {
        pthread_rwlock_unlock(&server->prompt_lock);
        return -1;
    }

    /* Check for duplicate name */
    for (int i = 0; i < server->prompt_count; i++) {
        if (strcmp(server->prompts[i].name, name) == 0) {
            pthread_rwlock_unlock(&server->prompt_lock);
            return -1;
        }
    }

    mcp_prompt_t *prompt = &server->prompts[server->prompt_count];
    strncpy(prompt->name, name, sizeof(prompt->name) - 1);
    strncpy(prompt->description, description ? description : "",
            sizeof(prompt->description) - 1);
    
    prompt->arg_count = arg_count;
    if (args && arg_count > 0) {
        memcpy(prompt->args, args, arg_count * sizeof(mcp_prompt_arg_t));
    }
    
    prompt->get_callback = get_callback;
    prompt->user_data = user_data;

    server->prompt_count++;
    pthread_rwlock_unlock(&server->prompt_lock);

    return 0;
}

int mcp_prompt_unregister(mcp_full_server_t *server, const char *name)
{
    if (!server || !name) {
        return -1;
    }

    pthread_rwlock_wrlock(&server->prompt_lock);

    for (int i = 0; i < server->prompt_count; i++) {
        if (strcmp(server->prompts[i].name, name) == 0) {
            memmove(&server->prompts[i],
                   &server->prompts[i + 1],
                   (server->prompt_count - i - 1) * sizeof(mcp_prompt_t));
            server->prompt_count--;
            pthread_rwlock_unlock(&server->prompt_lock);
            return 0;
        }
    }

    pthread_rwlock_unlock(&server->prompt_lock);
    return -1;
}

char *mcp_prompt_list(mcp_full_server_t *server)
{
    if (!server) {
        return NULL;
    }

    pthread_rwlock_rdlock(&server->prompt_lock);

    cJSON *root = cJSON_CreateObject();
    cJSON *prompts_array = cJSON_CreateArray();

    for (int i = 0; i < server->prompt_count; i++) {
        mcp_prompt_t *prompt = &server->prompts[i];
        
        cJSON *prompt_obj = cJSON_CreateObject();
        cJSON_AddStringToObject(prompt_obj, "name", prompt->name);
        cJSON_AddStringToObject(prompt_obj, "description", prompt->description);
        
        /* Add arguments */
        if (prompt->arg_count > 0) {
            cJSON *args_array = cJSON_CreateArray();
            for (int j = 0; j < prompt->arg_count; j++) {
                cJSON *arg_obj = cJSON_CreateObject();
                cJSON_AddStringToObject(arg_obj, "name", prompt->args[j].name);
                cJSON_AddStringToObject(arg_obj, "description",
                                       prompt->args[j].description);
                cJSON_AddBoolToObject(arg_obj, "required",
                                     prompt->args[j].required);
                cJSON_AddItemToArray(args_array, arg_obj);
            }
            cJSON_AddItemToObject(prompt_obj, "arguments", args_array);
        }
        
        cJSON_AddItemToArray(prompts_array, prompt_obj);
    }

    cJSON_AddItemToObject(root, "prompts", prompts_array);
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    pthread_rwlock_unlock(&server->prompt_lock);
    return json;
}

char *mcp_prompt_get(mcp_full_server_t *server,
                    const char *name,
                    const char *args_json)
{
    if (!server || !name) {
        return NULL;
    }

    pthread_rwlock_rdlock(&server->prompt_lock);

    for (int i = 0; i < server->prompt_count; i++) {
        if (strcmp(server->prompts[i].name, name) == 0) {
            mcp_prompt_t *prompt = &server->prompts[i];
            
            int message_count = 0;
            mcp_prompt_message_t *messages = prompt->get_callback(name,
                                                                 args_json,
                                                                 &message_count,
                                                                 prompt->user_data);
            
            pthread_rwlock_unlock(&server->prompt_lock);
            
            if (!messages || message_count == 0) {
                return NULL;
            }

            /* Create JSON response */
            cJSON *root = cJSON_CreateObject();
            cJSON *messages_array = cJSON_CreateArray();
            
            for (int j = 0; j < message_count; j++) {
                cJSON *msg_obj = cJSON_CreateObject();
                cJSON_AddStringToObject(msg_obj, "role", messages[j].role);
                cJSON *content_obj = cJSON_CreateObject();
                cJSON_AddStringToObject(content_obj, "type", "text");
                cJSON_AddStringToObject(content_obj, "text", messages[j].content);
                cJSON_AddItemToObject(msg_obj, "content", content_obj);
                cJSON_AddItemToArray(messages_array, msg_obj);
                
                free(messages[j].content);
            }
            free(messages);
            
            cJSON_AddItemToObject(root, "messages", messages_array);
            char *json = cJSON_PrintUnformatted(root);
            cJSON_Delete(root);
            
            return json;
        }
    }

    pthread_rwlock_unlock(&server->prompt_lock);
    return NULL;
}

/****************************************************************************
 * JSON-RPC 2.0
 ****************************************************************************/

mcp_jsonrpc_request_t *mcp_jsonrpc_parse_request(const char *json)
{
    if (!json) {
        return NULL;
    }

    cJSON *root = cJSON_Parse(json);
    if (!root) {
        return NULL;
    }

    mcp_jsonrpc_request_t *req = calloc(1, sizeof(mcp_jsonrpc_request_t));
    if (!req) {
        cJSON_Delete(root);
        return NULL;
    }

    /* Parse jsonrpc version */
    cJSON *jsonrpc = cJSON_GetObjectItem(root, "jsonrpc");
    if (jsonrpc && cJSON_IsString(jsonrpc)) {
        req->jsonrpc = strdup(jsonrpc->valuestring);
    }

    /* Parse method */
    cJSON *method = cJSON_GetObjectItem(root, "method");
    if (method && cJSON_IsString(method)) {
        req->method = strdup(method->valuestring);
    }

    /* Parse params */
    cJSON *params = cJSON_GetObjectItem(root, "params");
    if (params) {
        char *params_str = cJSON_PrintUnformatted(params);
        req->params = params_str;
    }

    /* Parse id */
    cJSON *id = cJSON_GetObjectItem(root, "id");
    if (id) {
        if (cJSON_IsString(id)) {
            req->id = strdup(id->valuestring);
        } else if (cJSON_IsNumber(id)) {
            char id_buf[32];
            snprintf(id_buf, sizeof(id_buf), "%d", id->valueint);
            req->id = strdup(id_buf);
        }
    }

    cJSON_Delete(root);
    return req;
}

char *mcp_jsonrpc_create_response(const char *id,
                                 const char *result,
                                 mcp_jsonrpc_error_t *error)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "jsonrpc", "2.0");

    if (error) {
        cJSON *error_obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(error_obj, "code", error->code);
        cJSON_AddStringToObject(error_obj, "message", error->message);
        if (error->data) {
            cJSON_AddStringToObject(error_obj, "data", error->data);
        }
        cJSON_AddItemToObject(root, "error", error_obj);
    } else if (result) {
        cJSON *result_obj = cJSON_Parse(result);
        if (result_obj) {
            cJSON_AddItemToObject(root, "result", result_obj);
        } else {
            cJSON_AddStringToObject(root, "result", result);
        }
    }

    if (id) {
        cJSON_AddStringToObject(root, "id", id);
    } else {
        cJSON_AddNullToObject(root, "id");
    }

    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json;
}

mcp_jsonrpc_error_t mcp_jsonrpc_error(int code,
                                     const char *message,
                                     const char *data)
{
    mcp_jsonrpc_error_t error;
    error.code = code;
    error.message = (char *)message;
    error.data = (char *)data;
    return error;
}

void mcp_jsonrpc_free_request(mcp_jsonrpc_request_t *req)
{
    if (!req) {
        return;
    }

    free(req->jsonrpc);
    free(req->method);
    free(req->params);
    free(req->id);
    free(req);
}

void mcp_jsonrpc_free_response(char *resp)
{
    free(resp);
}

/****************************************************************************
 * Protocol Handler
 ****************************************************************************/

char *mcp_handle_request(mcp_full_server_t *server, const char *request_json)
{
    if (!server || !request_json) {
        mcp_jsonrpc_error_t error = mcp_jsonrpc_error(
            MCP_ERROR_INVALID_REQUEST,
            "Invalid request",
            NULL);
        return mcp_jsonrpc_create_response(NULL, NULL, &error);
    }

    mcp_jsonrpc_request_t *req = mcp_jsonrpc_parse_request(request_json);
    if (!req || !req->method) {
        mcp_jsonrpc_error_t error = mcp_jsonrpc_error(
            MCP_ERROR_PARSE_ERROR,
            "Failed to parse request",
            NULL);
        char *resp = mcp_jsonrpc_create_response(
            req ? req->id : NULL, NULL, &error);
        mcp_jsonrpc_free_request(req);
        return resp;
    }

    /* Validate JSON-RPC version */
    if (!req->jsonrpc || strcmp(req->jsonrpc, "2.0") != 0) {
        mcp_jsonrpc_error_t error = mcp_jsonrpc_error(
            MCP_ERROR_INVALID_REQUEST,
            "Invalid JSON-RPC version",
            NULL);
        char *resp = mcp_jsonrpc_create_response(req->id, NULL, &error);
        mcp_jsonrpc_free_request(req);
        return resp;
    }

    char *response = NULL;

    /* Route to appropriate handler */
    if (strcmp(req->method, "tools/list") == 0) {
        response = mcp_handle_tools_list(server, req->id);
    } else if (strcmp(req->method, "tools/call") == 0) {
        response = mcp_handle_tools_call(server, req->id, req->params);
    } else if (strcmp(req->method, "resources/list") == 0) {
        response = mcp_handle_resources_list(server, req->id);
    } else if (strcmp(req->method, "resources/read") == 0) {
        response = mcp_handle_resources_read(server, req->id, req->params);
    } else if (strcmp(req->method, "prompts/list") == 0) {
        response = mcp_handle_prompts_list(server, req->id);
    } else if (strcmp(req->method, "prompts/get") == 0) {
        response = mcp_handle_prompts_get(server, req->id, req->params);
    } else {
        mcp_jsonrpc_error_t error = mcp_jsonrpc_error(
            MCP_ERROR_METHOD_NOT_FOUND,
            "Method not found",
            req->method);
        response = mcp_jsonrpc_create_response(req->id, NULL, &error);
    }

    mcp_jsonrpc_free_request(req);
    return response;
}

char *mcp_handle_tools_list(mcp_full_server_t *server, const char *id)
{
    char *tools_json = mcp_streaming_server_list_tools_json(&server->tool_server);
    char *response = mcp_jsonrpc_create_response(id, tools_json, NULL);
    free(tools_json);
    return response;
}

char *mcp_handle_tools_call(mcp_full_server_t *server,
                           const char *id,
                           const char *params)
{
    if (!params) {
        mcp_jsonrpc_error_t error = mcp_jsonrpc_error(
            MCP_ERROR_INVALID_PARAMS,
            "Missing parameters",
            NULL);
        return mcp_jsonrpc_create_response(id, NULL, &error);
    }

    cJSON *params_obj = cJSON_Parse(params);
    if (!params_obj) {
        mcp_jsonrpc_error_t error = mcp_jsonrpc_error(
            MCP_ERROR_INVALID_PARAMS,
            "Invalid parameters",
            NULL);
        return mcp_jsonrpc_create_response(id, NULL, &error);
    }

    cJSON *name = cJSON_GetObjectItem(params_obj, "name");
    cJSON *arguments = cJSON_GetObjectItem(params_obj, "arguments");
    
    if (!name || !cJSON_IsString(name)) {
        cJSON_Delete(params_obj);
        mcp_jsonrpc_error_t error = mcp_jsonrpc_error(
            MCP_ERROR_INVALID_PARAMS,
            "Missing tool name",
            NULL);
        return mcp_jsonrpc_create_response(id, NULL, &error);
    }

    char *args_json = arguments ? cJSON_PrintUnformatted(arguments) : strdup("{}");
    char *result = mcp_streaming_server_call_tool(&server->tool_server,
                                                  name->valuestring,
                                                  args_json,
                                                  NULL);
    free(args_json);
    cJSON_Delete(params_obj);

    char *response = mcp_jsonrpc_create_response(id, result, NULL);
    free(result);
    return response;
}

char *mcp_handle_resources_list(mcp_full_server_t *server, const char *id)
{
    char *resources_json = mcp_resource_list(server);
    char *response = mcp_jsonrpc_create_response(id, resources_json, NULL);
    free(resources_json);
    return response;
}

char *mcp_handle_resources_read(mcp_full_server_t *server,
                               const char *id,
                               const char *params)
{
    if (!params) {
        mcp_jsonrpc_error_t error = mcp_jsonrpc_error(
            MCP_ERROR_INVALID_PARAMS,
            "Missing parameters",
            NULL);
        return mcp_jsonrpc_create_response(id, NULL, &error);
    }

    cJSON *params_obj = cJSON_Parse(params);
    if (!params_obj) {
        mcp_jsonrpc_error_t error = mcp_jsonrpc_error(
            MCP_ERROR_INVALID_PARAMS,
            "Invalid parameters",
            NULL);
        return mcp_jsonrpc_create_response(id, NULL, &error);
    }

    cJSON *uri = cJSON_GetObjectItem(params_obj, "uri");
    if (!uri || !cJSON_IsString(uri)) {
        cJSON_Delete(params_obj);
        mcp_jsonrpc_error_t error = mcp_jsonrpc_error(
            MCP_ERROR_INVALID_PARAMS,
            "Missing resource URI",
            NULL);
        return mcp_jsonrpc_create_response(id, NULL, &error);
    }

    char *content = mcp_resource_read(server, uri->valuestring);
    cJSON_Delete(params_obj);

    if (!content) {
        mcp_jsonrpc_error_t error = mcp_jsonrpc_error(
            MCP_ERROR_INTERNAL_ERROR,
            "Failed to read resource",
            NULL);
        return mcp_jsonrpc_create_response(id, NULL, &error);
    }

    char *response = mcp_jsonrpc_create_response(id, content, NULL);
    free(content);
    return response;
}

char *mcp_handle_prompts_list(mcp_full_server_t *server, const char *id)
{
    char *prompts_json = mcp_prompt_list(server);
    char *response = mcp_jsonrpc_create_response(id, prompts_json, NULL);
    free(prompts_json);
    return response;
}

char *mcp_handle_prompts_get(mcp_full_server_t *server,
                             const char *id,
                             const char *params)
{
    if (!params) {
        mcp_jsonrpc_error_t error = mcp_jsonrpc_error(
            MCP_ERROR_INVALID_PARAMS,
            "Missing parameters",
            NULL);
        return mcp_jsonrpc_create_response(id, NULL, &error);
    }

    cJSON *params_obj = cJSON_Parse(params);
    if (!params_obj) {
        mcp_jsonrpc_error_t error = mcp_jsonrpc_error(
            MCP_ERROR_INVALID_PARAMS,
            "Invalid parameters",
            NULL);
        return mcp_jsonrpc_create_response(id, NULL, &error);
    }

    cJSON *name = cJSON_GetObjectItem(params_obj, "name");
    cJSON *arguments = cJSON_GetObjectItem(params_obj, "arguments");
    
    if (!name || !cJSON_IsString(name)) {
        cJSON_Delete(params_obj);
        mcp_jsonrpc_error_t error = mcp_jsonrpc_error(
            MCP_ERROR_INVALID_PARAMS,
            "Missing prompt name",
            NULL);
        return mcp_jsonrpc_create_response(id, NULL, &error);
    }

    char *args_json = arguments ? cJSON_PrintUnformatted(arguments) : NULL;
    char *result = mcp_prompt_get(server, name->valuestring, args_json);
    free(args_json);
    cJSON_Delete(params_obj);

    if (!result) {
        mcp_jsonrpc_error_t error = mcp_jsonrpc_error(
            MCP_ERROR_INTERNAL_ERROR,
            "Failed to get prompt",
            NULL);
        return mcp_jsonrpc_create_response(id, NULL, &error);
    }

    char *response = mcp_jsonrpc_create_response(id, result, NULL);
    free(result);
    return response;
}
