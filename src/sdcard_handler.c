#include "handlers.h"
#include "mime_type.h"
#include "path_utils.h"
#include "http_utils.h"
#include "mem_utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int is_sdcard_handler(http_request *request)
{
	// Security check for buffer overflow
	if (strlen(request->path) >= 1024) return 0;

	char decoded_path[1024];
	url_decode(decoded_path, request->path);

	if (strcmp(decoded_path, "/sdcard") == 0) return 1;
	if (startWith(decoded_path, "/sdcard/")) return 1;
	return 0;
}

http_response *handle_directory_listing(const char *path) {
	http_response *response = memalloc(sizeof(http_response));
    response->keep_alive = 0; // Initialize
    response->additional_headers = NULL; // Initialize
	DIR *d;
	struct dirent *dir;
	d = opendir(path);
	
	if (!d) {
		response->code = 404;
		char msg[1024];
		snprintf(msg, sizeof(msg), "404 Not Found (Dir): '%s'", path);
		response->payload = memdup(msg, strlen(msg));
		response->payload_len = strlen(msg);
		const char ct[] = "Content-Type: text/plain\r\n";
		response->content_type = memdup(ct, sizeof(ct));
		return response;
	}

	// Simple JSON construction buffer
	size_t buf_size = 16384;
	char *json = memalloc(buf_size);
	if (!json) { closedir(d); return NULL; }
	
	strcpy(json, "[");
	int first = 1;

	while ((dir = readdir(d)) != NULL) {
		if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;

		if (!first) strcat(json, ",");
		first = 0;

		char fullPath[512];
		// Avoid double slashes
		if (path[strlen(path)-1] == '/')
			snprintf(fullPath, sizeof(fullPath), "%s%s", path, dir->d_name);
		else
			snprintf(fullPath, sizeof(fullPath), "%s/%s", path, dir->d_name);
		
		struct stat st;
		const char *type = "file";
		size_t size = 0;

		if (stat(fullPath, &st) == 0) {
			if (S_ISDIR(st.st_mode)) type = "dir";
			size = st.st_size;
		} else {
			// Fallback if stat fails (e.g. permission issues or path length)
			// Try to use d_type if available
			#ifdef _DIRENT_HAVE_D_TYPE
			if (dir->d_type == DT_DIR) type = "dir";
			else if (dir->d_type == DT_UNKNOWN) type = "unknown";
			#endif
		}
		
		char entry[512];
		snprintf(entry, sizeof(entry), "{\"name\":\"%s\",\"type\":\"%s\",\"size\":%lu}", 
			dir->d_name, type, (unsigned long)size);
		
		if (strlen(json) + strlen(entry) < buf_size - 10) {
			strcat(json, entry);
		} else {
			break;
		}
	}
	strcat(json, "]");
	closedir(d);

	response->code = 200;
	response->payload = json;
	response->payload_len = strlen(json);
	const char ct[] = "Content-Type: application/json\r\n";
	response->content_type = memdup(ct, sizeof(ct));
    
    // Directory listings should not be cached or keep-alive
    response->keep_alive = 0; 
    response->additional_headers = NULL;

	return response;
}

http_response *handle_file_upload(http_request *request, const char *path) {
	http_response *response = memalloc(sizeof(http_response));
    response->keep_alive = 0; // Initialize
    response->additional_headers = NULL; // Initialize
	
    remove(path); // Workaround for 3DS FATfs not always truncating with "wb"
	FILE *f = fopen(path, "wb");
	if (!f) {
		response->code = 500;
		char msg[1024];
		snprintf(msg, sizeof(msg), "500 Write Error: '%s'", path);
		response->payload = memdup(msg, strlen(msg));
		response->payload_len = strlen(msg);
		return response;
	}

	long total_written = 0;

	// Write initial body part
	if (request->initial_body_len > 0 && request->body_start) {
		fwrite(request->body_start, 1, request->initial_body_len, f);
		total_written += request->initial_body_len;
	}

	// Read rest from socket
	if (total_written < request->content_length) {
		char *buf = memalloc(8192);
		if (buf) {
			while (total_written < request->content_length) {
				size_t to_read = 8192;
				if (request->content_length - total_written < 8192)
					to_read = request->content_length - total_written;
				
				ssize_t r = recv(request->client_id, buf, to_read, 0);
				if (r <= 0) break; // Error or disconnect
				
				fwrite(buf, 1, r, f);
				total_written += r;
			}
			free(buf);
		}
	}

	fclose(f);

	if (total_written == request->content_length) {
		response->code = 201;
		const char msg[] = "201 Created";
		response->payload = memdup(msg, sizeof(msg)-1);
		response->payload_len = sizeof(msg)-1;
	} else {
		response->code = 502; // Bad Gateway / Network Error
		char msg[128];
		snprintf(msg, sizeof(msg), "502 Incomplete: %ld/%ld bytes", total_written, request->content_length);
		response->payload = memdup(msg, strlen(msg));
		response->payload_len = strlen(msg);
	}

	const char ct[] = "Content-Type: text/plain\r\n";
	response->content_type = memdup(ct, sizeof(ct));
	return response;
}

http_response *get_sdcard_response(http_request *request)
{
	// Security check for buffer overflow before decoding
	if (strlen(request->path) >= 1024) {
		http_response *response = memalloc(sizeof(http_response));
		response->keep_alive = 0; // Initialize
		response->additional_headers = NULL; // Initialize
		response->code = 414; // URI Too Long
		const char msg[] = "414 URI Too Long";
		response->payload = memdup(msg, sizeof(msg)-1);
		response->payload_len = sizeof(msg)-1;
		return response;
	}

	char decoded_path[1024];
	url_decode(decoded_path, request->path);

	if (contains_path_traversal(decoded_path)) {
		http_response *response = memalloc(sizeof(http_response));
		response->keep_alive = 0; // Initialize
		response->additional_headers = NULL; // Initialize
		response->code = 403; 
		const char msg[] = "403 Forbidden";
		response->payload = memdup(msg, sizeof(msg)-1);
		response->payload_len = sizeof(msg)-1;
		return response;
	}

	const char *realPath = NULL;
	char *firstSlash = strchr(decoded_path, '/');
	if (firstSlash) {
		char *secondSlash = strchr(firstSlash + 1, '/');
		if (secondSlash) realPath = secondSlash;
	}

	if (!realPath) realPath = "/"; // Root

	// Handle Upload (PUT/POST)
	if (request->type == PUT || request->type == POST) {
		return handle_file_upload(request, realPath);
	}

	struct stat st;
	if (stat(realPath, &st) == 0 && S_ISDIR(st.st_mode)) {
		return handle_directory_listing(realPath);
	}

	// Regular File Download
	http_response *response = memalloc(sizeof(http_response));
    response->keep_alive = 0; // Initialize
    response->additional_headers = NULL; // Initialize
	FILE *fptr = fopen(realPath, "rb");
	
	if (fptr == NULL) {
		response->code = 404;
		char msg[1024];
		snprintf(msg, sizeof(msg), "404 Not Found (File): '%s'", realPath);
		response->payload = memdup(msg, strlen(msg));
		response->payload_len = strlen(msg);
		const char ct[] = "Content-Type: text/plain\r\n";
		response->content_type = memdup(ct, sizeof(ct));
	} else {
		fseek(fptr, 0, SEEK_END);
		size_t fileSize = ftell(fptr);
		fseek(fptr, 0, SEEK_SET);

		char *buffer = (char *)memalloc(fileSize);
		if (buffer) {
			size_t bytesRead = fread(buffer, 1, fileSize, fptr);
			response->payload = buffer;
			response->payload_len = bytesRead;
			response->code = 200;
		} else {
			response->code = 500;
			response->payload = NULL;
			response->payload_len = 0;
		}
		
		fclose(fptr);

		const char *mime = get_mime_type(realPath);
		char ct_buf[128];
		snprintf(ct_buf, sizeof(ct_buf), "Content-Type: %s\r\n", mime);
		response->content_type = memdup(ct_buf, strlen(ct_buf) + 1);

        // Add Cache-Control header for static files (e.g., 1 hour)
        // memdup will allocate memory for this header. It needs to be freed later.
        response->additional_headers = memdup("Cache-Control: public, max-age=3600\r\n", strlen("Cache-Control: public, max-age=3600\r\n") + 1);
	}
    
    // Set keep_alive true for successful file downloads for performance
    if (response->code == 200) {
        response->keep_alive = 1;
    }
	
	return response;    
}
