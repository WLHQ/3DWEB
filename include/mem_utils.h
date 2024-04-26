#ifndef MEM_UTILS_H
#define MEM_UTILS_H
#include <3ds.h>

void *memalloc(size_t size);
void *memdup(const void *data, size_t size);
void memdel(void **data);

#endif
