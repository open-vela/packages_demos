/****************************************************************************
 * src/conversation/mcp/core/mcp_stdio_transport.c
 * MCP stdio Transport Layer (NuttX)
 * 
 * Implements standard input/output communication for MCP protocol
 ****************************************************************************/

#include "mcp_server_unified.h"
// #include "mcp_protocol_full.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nuttx/pthread.h>
#include <unistd.h>

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct {
    mcp_full_server_t *server;
    pthread_t thread;
    bool running;
    FILE *input;
    FILE *output;
} g_stdio_server;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void *mcp_stdio_thread(void *arg)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fprintf(stderr, "[MCP STDIO] Server started\n");
    fflush(stderr);

    while (g_stdio_server.running) {
        /* Read JSON-RPC request from stdin */
        read = getline(&line, &len, g_stdio_server.input);
        if (read <= 0) {
            break;
        }

        /* Remove newline */
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }

        fprintf(stderr, "[MCP STDIO] Received: %s\n", line);

        /* Handle request */
        char *response = mcp_handle_request(g_stdio_server.server, line);
        
        if (response) {
            /* Write JSON-RPC response to stdout */
            fprintf(g_stdio_server.output, "%s\n", response);
            fflush(g_stdio_server.output);
            
            fprintf(stderr, "[MCP STDIO] Sent: %s\n", response);
            
            free(response);
        }
    }

    free(line);
    fprintf(stderr, "[MCP STDIO] Server stopped\n");
    return NULL;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int mcp_stdio_server_start(mcp_full_server_t *server)
{
    if (!server) {
        return -1;
    }

    if (g_stdio_server.running) {
        return -1;  /* Already running */
    }

    g_stdio_server.server = server;
    g_stdio_server.input = stdin;
    g_stdio_server.output = stdout;
    g_stdio_server.running = true;

    /* Create stdio thread */
    if (pthread_create(&g_stdio_server.thread, NULL,
                      mcp_stdio_thread, NULL) != 0) {
        g_stdio_server.running = false;
        return -1;
    }

    return 0;
}

void mcp_stdio_server_stop(void)
{
    if (!g_stdio_server.running) {
        return;
    }

    g_stdio_server.running = false;
    pthread_cancel(g_stdio_server.thread);
    pthread_join(g_stdio_server.thread, NULL);
}
