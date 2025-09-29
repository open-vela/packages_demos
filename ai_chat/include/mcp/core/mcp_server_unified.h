/****************************************************************************
 * src/conversation/mcp/mcp_server_unified.h
 * Unified MCP Server - Production Ready
 * 
 * Complete MCP Protocol Support:
 * - Tools (standard + streaming)
 * - Resources
 * - Prompts
 * - JSON-RPC 2.0
 * - stdio transport
 ****************************************************************************/
#ifndef __MCP_SERVER_UNIFIED_H
#define __MCP_SERVER_UNIFIED_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

/****************************************************************************
 * Configuration
 ****************************************************************************/

#define MCP_MAX_TOOLS      64
#define MCP_MAX_RESOURCES  32
#define MCP_MAX_PROMPTS    16
#define MCP_MAX_PARAMS     8

#define MCP_NAME_LEN       64
#define MCP_DESC_LEN       256
#define MCP_URI_LEN        256

/****************************************************************************
 * Core Types
 ****************************************************************************/

/* Parameter definition */
typedef struct {
    char name[MCP_NAME_LEN];
    char description[MCP_DESC_LEN];
    bool required;
} mcp_param_t;

/* ==================== Tools ==================== */

/* Tool callback - returns JSON result (caller must free) */
typedef char *(*mcp_tool_fn)(const char *args_json, void *user_data);

typedef struct {
    char name[MCP_NAME_LEN];
    char description[MCP_DESC_LEN];
    mcp_param_t params[MCP_MAX_PARAMS];
    int param_count;
    
    mcp_tool_fn callback;
    void *user_data;
    
    bool is_streaming;
    uint64_t call_count;
} mcp_tool_t;

/* ==================== Resources ==================== */

/* Resource read callback - returns content (caller must free) */
typedef char *(*mcp_resource_fn)(const char *uri, void *user_data);

typedef struct {
    char uri[MCP_URI_LEN];
    char name[MCP_NAME_LEN];
    char description[MCP_DESC_LEN];
    char mime_type[MCP_NAME_LEN];
    
    mcp_resource_fn read_callback;
    void *user_data;
} mcp_resource_t;

/* ==================== Prompts ==================== */

/* Prompt message */
typedef struct {
    char role[16];      /* "system", "user", "assistant" */
    char *content;      /* Content (caller must free) */
} mcp_prompt_message_t;

/* Prompt callback - returns message array (caller must free) */
typedef mcp_prompt_message_t *(*mcp_prompt_fn)(const char *name,
                                                const char *args_json,
                                                int *message_count,
                                                void *user_data);

typedef struct {
    char name[MCP_NAME_LEN];
    char description[MCP_DESC_LEN];
    mcp_param_t args[MCP_MAX_PARAMS];
    int arg_count;
    
    mcp_prompt_fn callback;
    void *user_data;
} mcp_prompt_t;

/* ==================== MCP Server ==================== */

typedef struct {
    /* Tools */
    mcp_tool_t tools[MCP_MAX_TOOLS];
    int tool_count;
    pthread_rwlock_t tool_lock;
    
    /* Resources */
    mcp_resource_t resources[MCP_MAX_RESOURCES];
    int resource_count;
    pthread_rwlock_t resource_lock;
    
    /* Prompts */
    mcp_prompt_t prompts[MCP_MAX_PROMPTS];
    int prompt_count;
    pthread_rwlock_t prompt_lock;
    
    /* Server info */
    char name[MCP_NAME_LEN];
    char version[32];
    
    /* stdio transport */
    pthread_t stdio_thread;
    bool stdio_running;
    
    bool initialized;
} mcp_server_t;

/****************************************************************************
 * Public API
 ****************************************************************************/

/* ==================== Server Lifecycle ==================== */

int mcp_server_init(mcp_server_t *server, const char *name, const char *version);
void mcp_server_destroy(mcp_server_t *server);

/* ==================== Tools ==================== */

int mcp_server_register_tool(mcp_server_t *server,
                              const char *name,
                              const char *description,
                              const mcp_param_t *params,
                              int param_count,
                              mcp_tool_fn callback,
                              void *user_data,
                              bool is_streaming);

/* ==================== Resources ==================== */

int mcp_server_register_resource(mcp_server_t *server,
                                  const char *uri,
                                  const char *name,
                                  const char *description,
                                  const char *mime_type,
                                  mcp_resource_fn callback,
                                  void *user_data);

/* ==================== Prompts ==================== */

int mcp_server_register_prompt(mcp_server_t *server,
                                const char *name,
                                const char *description,
                                const mcp_param_t *args,
                                int arg_count,
                                mcp_prompt_fn callback,
                                void *user_data);

/* ==================== Transport ==================== */

int mcp_server_start_stdio(mcp_server_t *server);
void mcp_server_stop_stdio(mcp_server_t *server);

/* ==================== Internal (for streaming tools) ==================== */

char *mcp_server_handle_request(mcp_server_t *server, const char *request_json);

#endif /* __MCP_SERVER_UNIFIED_H */
