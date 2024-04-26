#include "httpserver.h"
#include "mem_utils.h"

void *memalloc(size_t size)
{
	void *res = linearAlloc(size);
	if (!res)
		failExit("alloc: cannot allocate %d bytes!\n", size);
	else
		memset(res, 0, size);
	return (res);
}

void *memdup(const void *data, size_t size)
{
	void *res = memalloc(size);
	memcpy(res, data, size);
	return res;
}

void memdel(void **data)
{
	linearFree(*data);
	*data = NULL;
}
