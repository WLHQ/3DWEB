#include "mime_type.h"
#include <string.h>
#include <strings.h>

// Helper to determine MIME type based on extension
const char* get_mime_type(const char *path) {
	const char *ext = strrchr(path, '.');
	if (!ext) return "application/octet-stream";

	if (strcasecmp(ext, ".html") == 0) return "text/html";
	if (strcasecmp(ext, ".htm") == 0) return "text/html";
	if (strcasecmp(ext, ".css") == 0) return "text/css";
	if (strcasecmp(ext, ".js") == 0) return "application/javascript";
	if (strcasecmp(ext, ".json") == 0) return "application/json";
	if (strcasecmp(ext, ".png") == 0) return "image/png";
	if (strcasecmp(ext, ".jpg") == 0) return "image/jpeg";
	if (strcasecmp(ext, ".jpeg") == 0) return "image/jpeg";
	if (strcasecmp(ext, ".gif") == 0) return "image/gif";
	if (strcasecmp(ext, ".webp") == 0) return "image/webp";
	if (strcasecmp(ext, ".ico") == 0) return "image/x-icon";
	if (strcasecmp(ext, ".txt") == 0) return "text/plain";
	if (strcasecmp(ext, ".xml") == 0) return "text/xml";
	if (strcasecmp(ext, ".pdf") == 0) return "application/pdf";
	
	// Audio types
	if (strcasecmp(ext, ".ogg") == 0) return "audio/ogg";
	if (strcasecmp(ext, ".mp3") == 0) return "audio/mpeg";
	if (strcasecmp(ext, ".wav") == 0) return "audio/wav";
	
	// Video types
	if (strcasecmp(ext, ".mp4") == 0) return "video/mp4";
	if (strcasecmp(ext, ".webm") == 0) return "video/webm";
	if (strcasecmp(ext, ".mkv") == 0) return "video/x-matroska";

	return "application/octet-stream";
}
