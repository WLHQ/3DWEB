#include "handlers.h"

int	is_default_page(http_request *request)
{
	return strcmp(request->path, "/") == 0;
}

http_response *get_default_page(http_request *request)
{
	http_response *response = memalloc(sizeof(http_response));
	response->code = 200;
	const char content_type[] = "Content-Type: text/html\r\n";
	response->content_type = memdup(content_type, sizeof(content_type));
	char	*payload = memalloc(1024 * sizeof(char));
	sprintf(payload, "<html><title>Homepage</title><h1>Hello from your 3DS!</h1><p>I'm running firmware %lu</p><p>Visit <a href=\"/sdcard/Websites/index.html\">Index</a></p></html>", osGetFirmVersion());
	response->payload = payload;
     response->payload_len = strlen(response->payload);
	return response;
}
