#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H
#include <string.h>
#include "utils.h"
#include "http_types.h"

http_request_handler *get_request_handler(http_request *request);
http_request_type get_type(char *str);
char *get_browser(char *agent);
char *get_request_name(http_request_type type);
char *get_http_code_name(int code);

#endif
