#include "handlers.h"
#include "mime_type.h"
#include "path_utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h> // Für strcasecmp

int is_sdcard_handler(http_request *request)
{
	// Check for path starting with /sdcard/
	if (!(startWith(request->path, "/sdcard/") && strlen(request->path) > 8)) 
		return 0;

	return 1;
}

http_response *get_sdcard_response(http_request *request)
{
	http_response *response = memalloc(sizeof(http_response));
	if (!response) return NULL;

	if (contains_path_traversal(request->path)) {
		response->code = 403; // Forbidden
		const char msg[] = "403 Forbidden";
		response->payload = memdup(msg, sizeof(msg)-1);
		response->payload_len = sizeof(msg)-1;
		const char ct[] = "Content-Type: text/plain\r\n";
		response->content_type = memdup(ct, sizeof(ct)-1);
		return response;
	}

	// Logic from original: extract path after /sdcard
	// request->path: /sdcard/Websites/file.ext
	// Wanted: /Websites/file.ext (absolute path on SDMC root usually needs explicit sdmc:/ or just / if chdir was done)
	
	const char *realPath = NULL;
	// Original logic: strchr(strchr(path, '/') + 1, '/')
	// If path is "/sdcard/foo", result is "/foo"
	char *firstSlash = strchr(request->path, '/');
	if (firstSlash) {
		char *secondSlash = strchr(firstSlash + 1, '/');
		if (secondSlash) {
			realPath = secondSlash;
		}
	}

	if (!realPath) {
		// Fallback or Error
		response->code = 400; // Bad Request
		const char msg[] = "400 Bad Request";
		response->payload = memdup(msg, sizeof(msg)-1);
		response->payload_len = sizeof(msg)-1;
		const char ct[] = "Content-Type: text/plain\r\n";
		response->content_type = memdup(ct, sizeof(ct)-1);
		return response;
	}
	
	// Try opening the file
	// Note: We use "rb" for binary compatibility!
	FILE *fptr = fopen(realPath, "rb");
	
	if (fptr == NULL)
	{
		response->code = 404;
		const char msg[] = "404 Not Found";
		response->payload = memdup(msg, sizeof(msg)-1);
		response->payload_len = sizeof(msg)-1;
		const char ct[] = "Content-Type: text/plain\r\n";
		response->content_type = memdup(ct, sizeof(ct)-1);
	}
	else
	{
		fseek(fptr, 0, SEEK_END);
		size_t fileSize = ftell(fptr);
		fseek(fptr, 0, SEEK_SET);

		char *buffer = (char *)memalloc(fileSize);
		if (buffer) {
			size_t bytesRead = fread(buffer, 1, fileSize, fptr);
			// We trust bytesRead matches fileSize mostly, or we handle partial read?
			// For simplicity, we just send what we got.
			response->payload = buffer;
			response->payload_len = bytesRead;
			response->code = 200;
		} else {
			response->code = 500;
			response->payload = NULL;
			response->payload_len = 0;
		}
		
		fclose(fptr);

		// Dynamic Content-Type
		const char *mime = get_mime_type(realPath);
		char ct_buf[128];
		snprintf(ct_buf, sizeof(ct_buf), "Content-Type: %s\r\n", mime);
		response->content_type = memdup(ct_buf, strlen(ct_buf));
	}
	
	return response;    
}