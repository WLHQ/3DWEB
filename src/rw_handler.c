#include "handlers.h"

#define MAX_RW_BYTES 0x100

static int is_hex_char(char c)
{
	return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9');
}

static unsigned char decode_hex(char c)
{
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= '0' && c <= '9')
		return c - '0';
	
	return 0; // pls
}

int is_read_request(http_request *request)
{
	if (!startWith(request->path, "/readmem/") || strlen(request->path) < 9) // Ensure valid tokenization
		return 0;
		
	char *dup = strdup(request->path);
	int numParams = 0;
	int valid = 1;
	int l = 0;
	uint32_t addr;
	
	char *p = strtok(dup+1, "/");
	
	p = strtok(NULL, "/"); // "readmem/"
	
	while (p != NULL)
	{
		numParams++;
		switch(numParams)
		{
			case 1:
				addr = strtoul(p, NULL, 16);
				if (addr == 0 || addr == 0xFFFFFFFF)
					valid = 0;
				break;
			case 2:
				l = atoi(p);
				if (l <= 0 || l > 256)
					valid = 0;
				break;
			default:
				valid = 0;
				break;
		}
		p = strtok(NULL, "/");
	}
	
	if (numParams != 2)
		valid = 0;
	
	free(dup);
	return valid;

}

static int do_read_request(char *path, char *outbuf)
{        
	char *dup = strdup(path);
	int numParams = 0;
	int l = 0;
	uint32_t addr;
	
	char *p = strtok(dup+1, "/");
	
	p = strtok(NULL, "/"); // "readmem/"
	
	while (p != NULL)
	{
		numParams++;
		switch(numParams)
		{
			case 1:
				addr = strtoul(p, NULL, 16);
				break;
			case 2:
				l = atoi(p);
				break;
			default:
				break;
		}
		p = strtok(NULL, "/");
	}
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
	printTop("Reading %d bytes from 0x%08lX\n", l, (uint32_t)addr);
	#pragma GCC diagnostic pop
	for (int i = 0; i < l; i++)
	{
		((uint8_t *)outbuf)[i] = ((uint8_t *)addr)[i];
	}
	
	free(dup);
	return l;
}

http_response *get_read_handler_response(http_request *request)
{
	http_response *response = memalloc(sizeof(http_response));
	response->code = 200;
	response->content_type = strdup("Content-Type: application/octet-stream\r\n");
	char	*payload = memalloc(1024 * sizeof(char));
	int len = do_read_request(request->path, payload);
	response->payload = payload;
	response->payload_len = len;
	return response;    
}

int is_write_request(http_request *request)
{
	if (!startWith(request->path, "/writemem/") || strlen(request->path) < 10) // Ensure valid tokenization
		return 0;
		
	char *dup = strdup(request->path);
	int numParams = 0;
	int valid = 1;
	int l = 0;
	uint32_t addr;
	
	char *p = strtok(dup+1, "/");
	
	p = strtok(NULL, "/"); // "readmem/"
	
	while (p != NULL)
	{
		numParams++;
		switch(numParams)
		{
			case 1:
				addr = strtoul(p, NULL, 16);
				if (addr == 0 || addr == 0xFFFFFFFF)
					valid = 0;
				break;
			case 2:
				l = strlen(p);
				if (l % 2 != 0)
					valid = 0;
				if (l > 2 * MAX_RW_BYTES) // 0x100 bytes per request, maximum.
					valid = 0;
				char *temp = p;
				while (*temp)
				{
					if (!is_hex_char(*temp))
					{
						valid = 0;
						break;
					}
					temp++;
				}
				break;
			default:
				valid = 0;
				break;
		}
		p = strtok(NULL, "/");
	}
	
	if (numParams != 2)
		valid = 0;
	
	free(dup);
	return valid;
}

static int do_write_request(char *path)
{        
	char *dup = strdup(path);
	int numParams = 0;
	int l = 0;
	int buf_l = 0;
	int ofs = 0;
	uint32_t addr;
	uint8_t *ptr;
	
	char *p = strtok(dup+1, "/");
	
	p = strtok(NULL, "/"); // "readmem/"
	
	while (p != NULL)
	{
		numParams++;
		switch(numParams)
		{
			case 1:
				addr = strtoul(p, NULL, 16);
				ptr = (uint8_t *)addr;
				break;
			case 2:
				ofs = 0;
				l = strlen(p);
				buf_l = l/2;
				while (ofs < l)
				{
					#pragma GCC diagnostic push
					#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
					ptr[ofs/2] = (decode_hex(p[ofs]) << 4) | decode_hex(p[ofs+1]);
					#pragma GCC diagnostic pop
					ofs += 2;
				}
				break;
			default:
				break;
		}
		p = strtok(NULL, "/");
	}
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
	printTop("Wrote %d bytes from 0x%08lX\n", buf_l, (uint32_t)addr);
	#pragma GCC diagnostic pop
	
	free(dup);
	return l;
}

http_response *get_write_handler_response(http_request *request)
{
	do_write_request(request->path);
	
	http_response *response = memalloc(sizeof(http_response));
	response->code = 200;
	const char content_type[] = "Content-Type: text/html\r\n";
	response->content_type = memdup(content_type, sizeof(content_type));
	char	*payload = memalloc(1024 * sizeof(char));
	sprintf(payload, "OK");
	response->payload = payload;
	response->payload_len = strlen(response->payload);
	return response;
}

