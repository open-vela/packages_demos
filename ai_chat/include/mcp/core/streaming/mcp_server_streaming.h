/****************************************************************************
 * src/conversation/mcp/core/streaming/mcp_server_streaming.h
 * MCP Server with Streaming/SSE Support (NuttX)
 ****************************************************************************/
#ifndef __MCP_SERVER_STREAMING_H
#define __MCP_SERVER_STREAMING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <nuttx/pthread.h>

/* Import base types */
#define MCP_MAX_TOOLS 64
#define MCP_MAX_PARAMS 8
#define MCP_MAX_TOOL_NAME_LEN 64
#define MCP_MAX_TOOL_DESC_LEN 256

typedef struct {
    char name[64];
    char description[256];
    bool required;
} mcp_param_t;

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define MCP_STREAM_CHUNK_MAX_SIZE 4096

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Stream chunk type */
typedef enum {
    MCP_STREAM_CHUNK_DATA = 0,      /* Normal data chunk */
    MCP_STREAM_CHUNK_ERROR,          /* Error chunk */
    MCP_STREAM_CHUNK_DONE,           /* Stream completed */
    MCP_STREAM_CHUNK_METADATA        /* Metadata chunk */
} mcp_stream_chunk_type_t;

/* Stream chunk */
typedef struct {
    mcp_stream_chunk_type_t type;
    char *data;                      /* Chunk data (JSON or plain text) */
    size_t data_len;                 /* Data length */
    int sequence;                    /* Sequence number for ordering */
} mcp_stream_chunk_t;

/* Stream callback - called for each chunk */
typedef void (*mcp_stream_callback_t)(const mcp_stream_chunk_t *chunk, void *user_data);

/* Streaming tool callback signature */
typedef int (*mcp_streaming_tool_callback_t)(const char *params_json,
                                             mcp_stream_callback_t stream_cb,
                                             void *stream_user_data,
                                             void *tool_user_data);

/* Streaming tool definition */
typedef struct {
    char name[MCP_MAX_TOOL_NAME_LEN];
    char description[MCP_MAX_TOOL_DESC_LEN];
    mcp_param_t params[MCP_MAX_PARAMS];
    int param_count;
    mcp_streaming_tool_callback_t callback;
    void *user_data;
    bool is_streaming;               /* True if this tool supports streaming */
    
    /* Statistics */
    uint64_t call_count;
    uint64_t success_count;
    uint64_t error_count;
    uint64_t total_exec_time_us;
    uint64_t total_chunks_sent;
} mcp_streaming_tool_t;

/* Streaming server context */
typedef struct {
    mcp_streaming_tool_t tools[MCP_MAX_TOOLS];
    int tool_count;
    pthread_rwlock_t lock;
    bool initialized;
} mcp_streaming_server_t;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/* Server lifecycle */
int mcp_streaming_server_init(mcp_streaming_server_t *server);
void mcp_streaming_server_destroy(mcp_streaming_server_t *server);

/* Tool registration */
int mcp_streaming_server_register_tool(mcp_streaming_server_t *server,
                                       const char *name,
                                       const char *description,
                                       mcp_param_t *params,
                                       int param_count,
                                       mcp_streaming_tool_callback_t callback,
                                       void *user_data,
                                       bool is_streaming);

int mcp_streaming_server_unregister_tool(mcp_streaming_server_t *server,
                                         const char *name);

/* Tool execution with streaming support */
int mcp_streaming_server_call_tool(mcp_streaming_server_t *server,
                                   const char *tool_name,
                                   const char *params_json,
                                   mcp_stream_callback_t stream_cb,
                                   void *stream_user_data);

/* Tool query */
char *mcp_streaming_server_list_tools_json(mcp_streaming_server_t *server);

/* Utility functions for streaming tools */
mcp_stream_chunk_t mcp_stream_chunk_create(mcp_stream_chunk_type_t type,
                                           const char *data,
                                           int sequence);
void mcp_stream_chunk_free(mcp_stream_chunk_t *chunk);

/* Helper: Send a data chunk */
void mcp_stream_send_data(mcp_stream_callback_t cb, void *user_data,
                         const char *data, int sequence);

/* Helper: Send error chunk */
void mcp_stream_send_error(mcp_stream_callback_t cb, void *user_data,
                          const char *error, int sequence);

/* Helper: Send completion chunk */
void mcp_stream_send_done(mcp_stream_callback_t cb, void *user_data,
                         int sequence);

#endif /* __MCP_SERVER_STREAMING_H */
