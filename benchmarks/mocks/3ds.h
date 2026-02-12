#ifndef _3DS_MOCK_H
#define _3DS_MOCK_H

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

// Mock LightLock for utils.h if needed, though only pointers are used in headers usually?
// actually utils.h doesn't mention LightLock. src/utils.c does.
// But we are compiling src/http_utils.c.
// src/http_utils.c does not use LightLock.

#endif
