#include "httpserver.h"
#include "mem_utils.h"
#include <stdlib.h>

void *memalloc(size_t size)
{
	// Use standard calloc which is thread-safe in newlib and may be optimized
	// by the OS/allocator to return zeroed pages.
	return calloc(1, size);
}

void *memdup(const void *data, size_t size)
{
	// Use malloc directly to avoid redundant zeroing
	void *res = malloc(size);
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
