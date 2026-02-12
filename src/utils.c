#include "httpserver.h"

extern http_server *app_data;


// --------------------------------------------------------------------------
// console utils
int printTop(const char *format, ...)
{
	LightLock_Lock(&printLock);
	va_list ap;
	consoleSelect(&topScreen);
	va_start(ap, format);
	int res = vprintf(format, ap);
	va_end(ap);
	LightLock_Unlock(&printLock);
	return res;
}
int printBottom(const char *format, ...)
{
	LightLock_Lock(&printLock);
	va_list ap;
	consoleSelect(&bottomScreen);
	va_start(ap, format);
	int res = vprintf(format, ap);
	va_end(ap);
	LightLock_Unlock(&printLock);
	return res;
}

void clearBottom() {
	LightLock_Lock(&printLock);
	consoleSelect(&bottomScreen);
	consoleClear();
	LightLock_Unlock(&printLock);
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// string utils
int		startWith(char *str, char *start)
{
	if (!str || !start)
		return (0);
	return strncmp(str, start, strlen(start)) == 0;
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
	// Re-implementing simplified locking for failExit output
	LightLock_Lock(&printLock);
	consoleSelect(&topScreen);
	vprintf(fmt, ap);
	LightLock_Unlock(&printLock);
	
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