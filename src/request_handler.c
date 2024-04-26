#include "httpserver.h"

static http_request_handler_array	*request_handlers = NULL;

static http_request_handler_array	*alloc_entry()
{
	http_request_handler_array	*tmp = request_handlers;
	http_request_handler_array	*tmp2 = request_handlers;
	while (tmp2)
	{
		tmp = tmp2;
		tmp2 = tmp2->next;
	}

	tmp2 = memalloc(sizeof(http_request_handler_array));
	tmp2->data = memalloc(sizeof(http_request_handler));
	if (tmp)
		tmp->next = tmp2;
	else
		request_handlers = tmp2;
	return tmp2;
}
void register_handler(http_request_type type, is_handler check, compute_response get_response, situational_handle before_response, situational_handle after_response)
{
	http_request_handler_array	*entry = alloc_entry();
	if (entry)
	{
		entry->data->type = type;
		entry->data->check = check;
		entry->data->generate_response = get_response;
		entry->data->before_response = before_response;
		entry->data->after_response = after_response;
	}
}

http_request_handler *get_request_handler(http_request *req)
{
	http_request_handler_array	*tmp = request_handlers;
	while (tmp)
	{
		if (tmp->data->check(req))
			return tmp->data;
		tmp = tmp->next;
	}
	return NULL;
}
