#include "httpserver.h"
#include "mem_utils.h"
#include <stdlib.h>

void *memalloc(size_t size)
{
	// Use standard malloc which is thread-safe in newlib
	void *res = malloc(size);
	if (res)
		memset(res, 0, size);
	return (res);
}

void *memdup(const void *data, size_t size)
{
	void *res = memalloc(size);
	if (res)
		memcpy(res, data, size);
	return res;
}

void memdel(void **data)
{
	if (*data) {
		free(*data);
		*data = NULL;
	}
}
