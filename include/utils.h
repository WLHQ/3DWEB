#ifndef HTTP_SERVER_UTILS_H
#define HTTP_SERVER_UTILS_H
#include <3ds.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

int	printTop(const char *format, ...);
int	printBottom(const char *format, ...);
void clearBottom();

static inline int startWith(const char *str, const char *start)
{
	if (!str || !start)
		return (0);
	return strncmp(str, start, strlen(start)) == 0;
}

static inline int endsWith(const char *str, const char *suffix) {
    if (!str || !suffix)
        return 0;
    size_t len_str = strlen(str);
    size_t len_suffix = strlen(suffix);
    if (len_suffix > len_str)
        return 0;
    return strncmp(str + len_str - len_suffix, suffix, len_suffix) == 0;
}

__attribute__((format(printf,1,2)))
void failExit(const char *fmt, ...);
#endif
