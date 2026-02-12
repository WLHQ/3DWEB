#include "manual.h"
#include "utils.h"
#include <3ds.h>
#include <stdio.h>

void manual(int page) {
	const char *manual[8];
	manual[0] = "Loaded handlers:\n-Default Handler\n-Favicon Handler\n-System Handler\n-SDMC Handler\n-Memory R/W Handler\n-Encryption Handler\n-Note";
	manual[1] = "Default Handler\nPath: /\nDescription: Returns a hardcoded HTML response";
	manual[2] = "Favicon Handler\nPath: /favicon.ico\nDescription: Returns a hardcoded SVG favicon";
	manual[3] = "System Handler\nPath: /system/\nDescription: /system/exit - Exits app\n/system/reboot - Reboots system";
	manual[4] = "SDMC Handler\nPath: /sdcard/\nDescription: Returns content of requested file in SD Card with content-type text/html, if file is missing returns 404. Path starts at SDMC root";
	manual[5] = "Memory RW Handler\nPath: /readmem/ || /writemem/\nDescription: Reads and writes system memory.\nHonestly you're more likely to crash your 3DS. Check source code for details";
	manual[6] = "Encryption Handler\nPath: /crypt/\n Description: Uses native AES Encryption/Decryption Algorithms to do CBC/CTR/CTM. Check source code for details.";
	manual[7] = "Note:\n\nIf the console freezes while a client requests a page, you will be unable to switch tabs until the request is completed. During this time, any inputs for switching pages in the manual on the console, switching ports, or returning to the home menu will be unavailable. The console may also stay frozen when a page is loaded other than the hardcoded one. The console will unfreeze when the user requests the hardcoded HTML again.";
	clearBottom();
	printBottom("eManual page %i\n%s\n\nPress L for previous page or R for next page.\nPress X to change port or START to exit.",page,manual[page]);
	gfxFlushBuffers();
}
