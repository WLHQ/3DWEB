#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include <stdio.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "http_types.h"
#include "http_utils.h"
#include "mem_utils.h"
#include "utils.h"

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000

#define HTTP_HEADER_TEMPLATE "HTTP/1.1 %d %s\r\n"

// silence the unused warning because we use it!
__attribute__((unused))
static http_response DEFAULT_PAGE = {.code = 501, .content_type = "Content-Type: text/html\r\n", .payload = "<html><title>501 - Not Implemented</title><h1>501 - Not Implemented</h1></html>"};
extern PrintConsole topScreen, bottomScreen;
extern LightLock printLock; // Global lock for console output

typedef struct {
    bool is_new_3ds;
    u32 stack_size;
    u32 socket_buffer_size;
} SystemConfig;

extern SystemConfig sys_conf; // Global verfügbar machen

void init();
int loop();
void destroy();
void start_connection_thread(http_server *server_template, s32 client_id, struct sockaddr_in client_addr);
void register_handler(http_request_type type, is_handler check, compute_response get_response, situational_handle before_response, situational_handle after_response);
void init_handlers();
#endif
