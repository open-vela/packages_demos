/****************************************************************************
 * src/conversation/mcp/core/protocol/mcp_protocol_full.h
 * Complete MCP Protocol Implementation (NuttX)
 * 
 * Implements full MCP specification:
 * - Tools (tools/list, tools/call)
 * - Resources (resources/list, resources/read, resources/subscribe)
 * - Prompts (prompts/list, prompts/get)
 * - JSON-RPC 2.0 protocol layer
 * - stdio/SSE transport
 ****************************************************************************/
#ifndef __MCP_PROTOCOL_FULL_H
#define __MCP_PROTOCOL_FULL_H

#include "../streaming/mcp_server_streaming.h"
#include <stdint.h>
#include <stdbool.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define MCP_MAX_RESOURCES 64
#define MCP_MAX_PROMPTS 32
#define MCP_MAX_RESOURCE_URI_LEN 256
#define MCP_MAX_PROMPT_NAME_LEN 64
#define MCP_MAX_MIME_TYPE_LEN 64

/* JSON-RPC 2.0 Error Codes */
#define MCP_ERROR_PARSE_ERROR      -32700
#define MCP_ERROR_INVALID_REQUEST  -32600
#define MCP_ERROR_METHOD_NOT_FOUND -32601
#define MCP_ERROR_INVALID_PARAMS   -32602
#define MCP_ERROR_INTERNAL_ERROR   -32603

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* ==================== Resources ==================== */

/* Resource types */
typedef enum {
    MCP_RESOURCE_TYPE_FILE = 0,
    MCP_RESOURCE_TYPE_URL,
    MCP_RESOURCE_TYPE_BLOB,
    MCP_RESOURCE_TYPE_TEXT
} mcp_resource_type_t;

/* Resource definition */
typedef struct {
    char uri[MCP_MAX_RESOURCE_URI_LEN];
    char name[MCP_MAX_TOOL_NAME_LEN];
    char description[MCP_MAX_TOOL_DESC_LEN];
    char mime_type[MCP_MAX_MIME_TYPE_LEN];
    mcp_resource_type_t type;
    void *user_data;
    
    /* Resource read callback */
    char *(*read_callback)(const char *uri, void *user_data);
    
    /* Subscribe callback (optional for dynamic resources) */
    int (*subscribe_callback)(const char *uri, void *user_data);
} mcp_resource_t;

/* ==================== Prompts ==================== */

/* Prompt argument */
typedef struct {
    char name[MCP_MAX_PARAM_NAME_LEN];
    char description[MCP_MAX_PARAM_DESC_LEN];
    bool required;
} mcp_prompt_arg_t;

/* Prompt message */
typedef struct {
    char role[16];      /* "user", "assistant", "system" */
    char *content;      /* Message content */
} mcp_prompt_message_t;

/* Prompt definition */
typedef struct {
    char name[MCP_MAX_PROMPT_NAME_LEN];
    char description[MCP_MAX_TOOL_DESC_LEN];
    mcp_prompt_arg_t args[MCP_MAX_PARAMS];
    int arg_count;
    
    /* Prompt get callback */
    mcp_prompt_message_t *(*get_callback)(const char *name,
                                         const char *args_json,
                                         int *message_count,
                                         void *user_data);
    void *user_data;
} mcp_prompt_t;

/* ==================== JSON-RPC 2.0 ==================== */

/* JSON-RPC Request */
typedef struct {
    char *jsonrpc;      /* Must be "2.0" */
    char *method;       /* Method name */
    char *params;       /* JSON params string */
    char *id;          /* Request ID (NULL for notifications) */
} mcp_jsonrpc_request_t;

/* JSON-RPC Response */
typedef struct {
    char *jsonrpc;      /* Must be "2.0" */
    char *result;       /* Result JSON (NULL if error) */
    char *error;        /* Error JSON (NULL if success) */
    char *id;          /* Request ID */
} mcp_jsonrpc_response_t;

/* JSON-RPC Error */
typedef struct {
    int code;
    char *message;
    char *data;         /* Optional additional error data */
} mcp_jsonrpc_error_t;

/* ==================== Full MCP Server ==================== */

/* MCP Server with all features */
typedef struct {
    /* Tools (already implemented) */
    mcp_streaming_server_t tool_server;
    
    /* Resources */
    mcp_resource_t resources[MCP_MAX_RESOURCES];
    int resource_count;
    pthread_rwlock_t resource_lock;
    
    /* Prompts */
    mcp_prompt_t prompts[MCP_MAX_PROMPTS];
    int prompt_count;
    pthread_rwlock_t prompt_lock;
    
    /* Server info */
    char server_name[64];
    char server_version[32];
    
    bool initialized;
} mcp_full_server_t;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/* ==================== Server Lifecycle ==================== */

int mcp_full_server_init(mcp_full_server_t *server,
                         const char *name,
                         const char *version);
void mcp_full_server_destroy(mcp_full_server_t *server);

/* ==================== Resources API ==================== */

/* Register a resource */
int mcp_resource_register(mcp_full_server_t *server,
                         const char *uri,
                         const char *name,
                         const char *description,
                         const char *mime_type,
                         mcp_resource_type_t type,
                         char *(*read_callback)(const char *uri, void *user_data),
                         void *user_data);

/* Unregister a resource */
int mcp_resource_unregister(mcp_full_server_t *server, const char *uri);

/* List all resources (returns JSON) */
char *mcp_resource_list(mcp_full_server_t *server);

/* Read a resource */
char *mcp_resource_read(mcp_full_server_t *server, const char *uri);

/* Subscribe to resource updates */
int mcp_resource_subscribe(mcp_full_server_t *server, const char *uri);

/* ==================== Prompts API ==================== */

/* Register a prompt */
int mcp_prompt_register(mcp_full_server_t *server,
                       const char *name,
                       const char *description,
                       mcp_prompt_arg_t *args,
                       int arg_count,
                       mcp_prompt_message_t *(*get_callback)(const char *name,
                                                            const char *args_json,
                                                            int *message_count,
                                                            void *user_data),
                       void *user_data);

/* Unregister a prompt */
int mcp_prompt_unregister(mcp_full_server_t *server, const char *name);

/* List all prompts (returns JSON) */
char *mcp_prompt_list(mcp_full_server_t *server);

/* Get a prompt with arguments */
char *mcp_prompt_get(mcp_full_server_t *server,
                    const char *name,
                    const char *args_json);

/* ==================== JSON-RPC 2.0 ==================== */

/* Parse JSON-RPC request */
mcp_jsonrpc_request_t *mcp_jsonrpc_parse_request(const char *json);

/* Create JSON-RPC response */
char *mcp_jsonrpc_create_response(const char *id,
                                 const char *result,
                                 mcp_jsonrpc_error_t *error);

/* Create JSON-RPC error */
mcp_jsonrpc_error_t mcp_jsonrpc_error(int code,
                                     const char *message,
                                     const char *data);

/* Free JSON-RPC structures */
void mcp_jsonrpc_free_request(mcp_jsonrpc_request_t *req);
void mcp_jsonrpc_free_response(char *resp);

/* ==================== Protocol Handler ==================== */

/* Main protocol handler - processes JSON-RPC requests */
char *mcp_handle_request(mcp_full_server_t *server, const char *request_json);

/* Handle specific methods */
char *mcp_handle_tools_list(mcp_full_server_t *server, const char *id);
char *mcp_handle_tools_call(mcp_full_server_t *server,
                           const char *id,
                           const char *params);

char *mcp_handle_resources_list(mcp_full_server_t *server, const char *id);
char *mcp_handle_resources_read(mcp_full_server_t *server,
                               const char *id,
                               const char *params);

char *mcp_handle_prompts_list(mcp_full_server_t *server, const char *id);
char *mcp_handle_prompts_get(mcp_full_server_t *server,
                             const char *id,
                             const char *params);

/* ==================== Transport Layer ==================== */

/* stdio transport */
int mcp_stdio_server_start(mcp_full_server_t *server);
void mcp_stdio_server_stop(void);

/* SSE transport (HTTP endpoint) */
int mcp_sse_server_start(mcp_full_server_t *server, int port);
void mcp_sse_server_stop(void);

#endif /* __MCP_PROTOCOL_FULL_H */
