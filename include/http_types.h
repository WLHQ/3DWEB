#ifndef HTTP_TYPES_H
#define HTTP_TYPES_H

#include <3ds.h>
#include <netinet/in.h>

typedef struct
{
	u8 running;
	struct sockaddr_in client_addr;
	struct sockaddr_in server_addr;
	u32 client_length;
	s32 server_id;
	s32 client_id;
} http_server;

typedef enum
{
	GET,
	HEAD,
	POST,
	OPTIONS,
	CONNECT,
	TRACE,
	PUT,
	PATCH,
	DELETE,
	UNKNOWN
} http_request_type;

typedef struct
{
	http_request_type type;
	char *agent;
	char *header;
	char *hostname;
	char *path;
	char *payload;
} http_request;

typedef struct
{
	int code;
	char *content_type;
	int payload_len;
	char *payload;
} http_response;

typedef int (*is_handler)(http_request*);
typedef http_response *(*compute_response)(http_request*);
typedef void (*situational_handle)(http_server*, http_request*);

typedef struct
{
	http_request_type type;
	is_handler check;
	compute_response generate_response;
	situational_handle before_response;
	situational_handle after_response;
} http_request_handler;

typedef struct s_handler_array
{
	http_request_handler	*data;
	struct s_handler_array	*next;
} http_request_handler_array;
#endif
