#include "httpserver.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

// Helper struct for passing data to thread
typedef struct {
    s32 client_id;
    struct sockaddr_in client_addr;
    http_server server_ctx; // Copy of server state
} thread_context;

static void compute_path(http_request *request)
{
	// Default value
	request->path = "/";
	char *request_type = get_request_name(request->type);
	char *path = request->header;
	char *saveptr;

	// Unknown request type?
	if (request_type)
		path += strlen(request_type) + 1;
	path = strtok_r(path, " ", &saveptr);

	// If path isn't NULL
	if (path)
		request->path = path;
}

void send_response(s32 client_id, http_response *response)
{
	char headerBuffer[2048]; 
	memset(headerBuffer, 0, sizeof(headerBuffer));
	
	int len = 0;
	
	len += snprintf(headerBuffer + len, sizeof(headerBuffer) - len, 
	         HTTP_HEADER_TEMPLATE, response->code, get_http_code_name(response->code));
	
	if (response->content_type) {
		len += snprintf(headerBuffer + len, sizeof(headerBuffer) - len, "%s", response->content_type);
	}
	len += snprintf(headerBuffer + len, sizeof(headerBuffer) - len, "Server: 3ds-httpd\r\n");
	len += snprintf(headerBuffer + len, sizeof(headerBuffer) - len, "Connection: close\r\n");
	len += snprintf(headerBuffer + len, sizeof(headerBuffer) - len, "Content-Length: %u\r\n", response->payload_len);
	len += snprintf(headerBuffer + len, sizeof(headerBuffer) - len, "\r\n");
	
	send(client_id, headerBuffer, len, 0);

	if (response->payload && response->payload_len > 0) {
		send(client_id, response->payload, response->payload_len, 0);
	}
}

void handle_client(thread_context *ctx)
{
    // Local buffer for this thread
    char *payload = memalloc(4098);
    if (!payload) {
        close(ctx->client_id);
        free(ctx);
        return;
    }

    // Set socket timeout to 10 seconds to prevent zombie threads using select
    fd_set readfds;
    struct timeval tv;
    FD_ZERO(&readfds);
    FD_SET(ctx->client_id, &readfds);
    tv.tv_sec = 10;
    tv.tv_usec = 0;

    int select_ret = select(ctx->client_id + 1, &readfds, NULL, NULL, &tv);

    if (select_ret == 0) {
        // Timeout
        free(payload);
        close(ctx->client_id);
        free(ctx);
        return;
    } else if (select_ret < 0) {
        // Error
        free(payload);
        close(ctx->client_id);
        free(ctx);
        return;
    }

    // Read request
    ssize_t ret = recv(ctx->client_id, payload, 4096, 0); 
    
    if (ret <= 0) {
        free(payload);
        close(ctx->client_id);
        free(ctx);
        return;
    }

    http_request *request = memalloc(sizeof(http_request));
    if (!request) { free(payload); close(ctx->client_id); free(ctx); return; }

    request->payload = payload;
    char *saveptr;
    request->header = strtok_r(payload, "\r\n", &saveptr);
    if (!request->header) {
        memdel((void**)&request);
        free(payload);
        close(ctx->client_id);
        free(ctx);
        return;
    }

    request->type = get_type(request->header);

    char *rawData = strtok_r(NULL, "\r\n", &saveptr);
    while (rawData)
    {
        if (startWith(rawData, "Host: "))
            request->hostname = rawData + 6;
        else if (startWith(rawData, "User-Agent: "))
            request->agent = rawData + 12;
        rawData = strtok_r(NULL, "\r\n", &saveptr);
    }

    compute_path(request);

    http_request_handler *handler = get_request_handler(request);

    // Update server context wrapper
    ctx->server_ctx.client_id = ctx->client_id;
    ctx->server_ctx.client_addr = ctx->client_addr;

    if (handler && handler->before_response != NULL)
        handler->before_response(&ctx->server_ctx, request);

    http_response *response = NULL;
    if (handler)
        response = handler->generate_response(request);

    if (response == NULL)
    {
        response = memalloc(sizeof(http_response));
        if (response) {
            response->code = DEFAULT_PAGE.code;
            response->content_type = (char*)memdup(DEFAULT_PAGE.content_type, strlen(DEFAULT_PAGE.content_type));
            response->payload = (char*)memdup(DEFAULT_PAGE.payload, strlen(DEFAULT_PAGE.payload));
            response->payload_len = strlen(DEFAULT_PAGE.payload);
        }
    }

    if (response) {
        char ipStr[INET_ADDRSTRLEN]; // Buffer for thread-safe IP string
        inet_ntop(AF_INET, &(ctx->client_addr.sin_addr), ipStr, INET_ADDRSTRLEN);
        printTop("[%d]: %s %s (client: %s)\n", response->code, get_request_name(request->type), request->path, ipStr);
        send_response(ctx->client_id, response);

        memdel((void**)&response->content_type);
        memdel((void**)&response->payload);
        memdel((void**)&response);
    }

    if (handler && handler->after_response != NULL)
        handler->after_response(&ctx->server_ctx, request);

    memdel((void**)&request);
    free(payload);
    
    close(ctx->client_id);
    free(ctx);
}

void connection_thread_entry(void *arg)
{
    thread_context *ctx = (thread_context*)arg;
    handle_client(ctx);
}

void start_connection_thread(http_server *server_template, s32 client_id, struct sockaddr_in client_addr)
{
    thread_context *ctx = malloc(sizeof(thread_context));
    if (!ctx) {
        close(client_id);
        return;
    }
    
    ctx->client_id = client_id;
    ctx->client_addr = client_addr;
    memcpy(&ctx->server_ctx, server_template, sizeof(http_server));
    
    s32 prio = 0;
    svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
    
    // Create a detached thread with dynamic stack size
    Thread t = threadCreate(connection_thread_entry, ctx, sys_conf.stack_size, prio - 1, -2, true);
    if (!t) {
        close(client_id);
        free(ctx);
    }
}
