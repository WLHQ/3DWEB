#include "httpserver.h"

extern http_server *app_data;


// --------------------------------------------------------------------------
// console utils
int printTop(const char *format, ...)
{
	va_list ap;
	consoleSelect(&topScreen);
	va_start(ap, format);
	int res = vprintf(format, ap);
	va_end(ap);
	return res;
}
int printBottom(const char *format, ...)
{
	va_list ap;
	consoleSelect(&bottomScreen);
	va_start(ap, format);
	int res = vprintf(format, ap);
	va_end(ap);
	return res;
}

void clearBottom() {
	consoleSelect(&bottomScreen);
	consoleClear();
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// string utils
int						startWith(char *str, char *start)
{
	if (!str || !start)
		return (0);
	int startlen = strlen(start);
	return startlen <= strlen(str) && strncmp(str, start, startlen) == 0;
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// others utils
void failExit(const char *fmt, ...)
{

	if(app_data->server_id > 0) close(app_data->server_id);
	if(app_data->client_id > 0) close(app_data->client_id);

	va_list ap;

	printTop(CONSOLE_RED);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printTop(CONSOLE_RESET);
	printTop("\nPress B to exit\n");
	clearBottom();

	while (aptMainLoop()) {
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_B) exit(0);
	}
}
// --------------------------------------------------------------------------
