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
		response->content_type = memdup(ct, sizeof(ct)-1);
		return response;
	}

	// Simple JSON construction buffer
	size_t buf_size = 16384;
	char *json = memalloc(buf_size);
	if (!json) { closedir(d); return NULL; }
	
	size_t offset = 0;
	json[offset++] = '[';
	json[offset] = '\0';
	int first = 1;

	while ((dir = readdir(d)) != NULL) {
		if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;

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
		int entry_len = snprintf(entry, sizeof(entry), "%s{\"name\":\"%s\",\"type\":\"%s\",\"size\":%lu}",
			first ? "" : ",", dir->d_name, type, (unsigned long)size);
		
		if (entry_len > 0 && offset + entry_len < buf_size - 2) { // Reserve space for closing ']' and '\0'
			memcpy(json + offset, entry, entry_len);
			offset += entry_len;
			json[offset] = '\0';
			first = 0;
		} else {
			break;
		}
	}
	json[offset++] = ']';
	json[offset] = '\0';
	closedir(d);

	response->code = 200;
	response->payload = json;
	response->payload_len = offset;
	const char ct[] = "Content-Type: application/json\r\n";
	response->content_type = memdup(ct, sizeof(ct)-1);
    
    // Directory listings should not be cached or keep-alive
    response->keep_alive = 0; 
    response->additional_headers = NULL;

	return response;
}

// Handles DELETE requests for files on the SD card with security checks
http_response *handle_file_delete(http_request *request, const char *path) {
    http_response *response = memalloc(sizeof(http_response));
    response->keep_alive = 0; // Initialize
    response->additional_headers = NULL; // Initialize
    response->content_type = memdup("Content-Type: text/plain\r\n", sizeof("Content-Type: text/plain\r\n"));

    // SECURITY: Ensure path is within /sdcard/Websites/ and contains no traversal
    // The `path` argument here is already relative to /sdcard (e.g., /Websites/file.json)
    // We need to rebuild the full path for `remove` and then check against a base path.
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "sdmc:%s", path); // Reconstruct full path for `remove`

    // IMPORTANT: Restrict DELETE to the /sdcard/Websites/ directory
    // And specifically to files starting with "calendar_bk_" for safety or other specified pattern
    if (!startWith(path, "/Websites/calendar_bk_") || !endsWith(path, ".json") || contains_path_traversal(path)) {
        response->code = 403; // Forbidden
        response->payload = memdup("403 Forbidden: Deletion of this file is not allowed.", sizeof("403 Forbidden: Deletion of this file is not allowed."));
        response->payload_len = sizeof("403 Forbidden: Deletion of this file is not allowed.") - 1;
        return response;
    }
    
    // Check if the file exists before attempting to delete
    FILE *f_check = fopen(full_path, "rb");
    if (!f_check) {
        response->code = 404; // Not Found
        response->payload = memdup("404 Not Found: File does not exist.", sizeof("404 Not Found: File does not exist.") - 1);
        response->payload_len = sizeof("404 Not Found: File does not exist.") - 1;
        return response;
    }
    fclose(f_check); // Close immediately

    if (remove(full_path) == 0) {
        response->code = 200; // OK
        response->payload = memdup("200 OK: File deleted successfully.", sizeof("200 OK: File deleted successfully.") - 1);
        response->payload_len = sizeof("200 OK: File deleted successfully.") - 1;
    } else {
        response->code = 500; // Internal Server Error
        response->payload = memdup("500 Internal Server Error: Failed to delete file.", sizeof("500 Internal Server Error: Failed to delete file.") - 1);
        response->payload_len = sizeof("500 Internal Server Error: Failed to delete file.") - 1;
    }

    return response;
}

http_response *handle_file_upload(http_request *request, const char *path) {
	http_response *response = memalloc(sizeof(http_response));
    response->keep_alive = 0; // Initialize
    response->additional_headers = NULL; // Initialize
    response->content_type = memdup("Content-Type: text/plain\r\n", sizeof("Content-Type: text/plain\r\n"));
	
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "sdmc:%s", path); // Reconstruct full path

    // Security check: ensure path is within /sdcard/Websites/ or other allowed paths
    // Similar to handle_file_delete, prevent writing outside safe zones
    if (!startWith(path, "/Websites/") || contains_path_traversal(path)) {
        // Specific check for calendar_data.json and calendar_bk_*.json
        if (! (strcmp(path, "/Websites/calendar_data.json") == 0 || startWith(path, "/Websites/calendar_bk_"))) {
            response->code = 403; // Forbidden
            response->payload = memdup("403 Forbidden: Writing to this path is not allowed.", sizeof("403 Forbidden: Writing to this path is not allowed.") - 1);
            response->payload_len = sizeof("403 Forbidden: Writing to this path is not allowed.") - 1;
            return response;
        }
    }

    remove(full_path); // Workaround for 3DS FATfs not always truncating with "wb"
	FILE *f = fopen(full_path, "wb");
	if (!f) {
		response->code = 500;
		char msg[1024];
		snprintf(msg, sizeof(msg), "500 Write Error: '%.1004s'", full_path);
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

	// const char ct[] = "Content-Type: text/plain\r\n"; // Removed as content_type is set at the top.
	// response->content_type = memdup(ct, sizeof(ct)); 
	return response;
}

http_response *get_sdcard_response(http_request *request)
{
	// Security check for buffer overflow before decoding
	if (strlen(request->path) >= 1024) {
		http_response *response = memalloc(sizeof(http_response));
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
		response->code = 403; 
		const char msg[] = "403 Forbidden";
		response->payload = memdup(msg, strlen(msg));
		response->payload_len = strlen(msg);
		return response;
	}

	const char *realPath = NULL;
	char *firstSlash = strchr(decoded_path, '/');
	if (firstSlash) {
		char *secondSlash = strchr(firstSlash + 1, '/');
		if (secondSlash) realPath = secondSlash;
	}

	if (!realPath) realPath = "/"; // Root

    // Handle DELETE requests first
    if (request->type == DELETE) {
        return handle_file_delete(request, realPath);
    }
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
		fclose(fptr); // Close immediately, will be reopened for streaming

		response->payload = NULL; // No in-memory payload for streaming
		response->payload_len = fileSize;
		response->stream_file_path = memdup(realPath, strlen(realPath) + 1); // Store path for streaming
		response->code = 200;
		
		const char *mime = get_mime_type(realPath);
		char ct_buf[128];
		snprintf(ct_buf, sizeof(ct_buf), "Content-Type: %s\r\n", mime);
		response->content_type = memdup(ct_buf, strlen(ct_buf) + 1);

        response->additional_headers = memdup("Cache-Control: public, max-age=3600\r\n", strlen("Cache-Control: public, max-age=3600\r\n") + 1);
	}
    
    // Set keep_alive true for successful file downloads for performance
    if (response->code == 200) {
        response->keep_alive = 1;
    }
	
	return response;    
}