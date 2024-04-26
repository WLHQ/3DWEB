#ifndef HTTP_SERVER_UTILS_H
#define HTTP_SERVER_UTILS_H
#include <3ds.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

int	printTop(const char *format, ...);
int	printBottom(const char *format, ...);
void clearBottom();
int	startWith(char *str, char *start);

__attribute__((format(printf,1,2)))
void failExit(const char *fmt, ...);
#endif
