#ifndef HTTP_HANDLERS_H
#define HTTP_HANDLERS_H
#include "httpserver.h"

int is_default_page(http_request *request);
http_response *get_default_page(http_request *request);
int is_sdcard_handler(http_request *request);
http_response *get_sdcard_response(http_request *request);
int is_system_request(http_request *request);
http_response *get_system_handler_response(http_request *request);
void pre_system_response(http_server *server, http_request *request);

int is_crypt_request(http_request *request);
http_response *get_crypt_handler_response(http_request *request);

int is_read_request(http_request *request);
http_response *get_read_handler_response(http_request *request);

int is_write_request(http_request *request);
http_response *get_write_handler_response(http_request *request);
int is_favicon_request(http_request *request);
http_response *get_favicon_icon(http_request *request);
#endif
