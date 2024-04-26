#include "httpserver.h"
#include <3ds.h>
#include <stdio.h>
#include <string.h>
void manual(int page) {
	const char *manual[8];
	//TODO: Move manual pages into handlers
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
int	main(int ac, char **av)
{
	
	int port = 8081;
	int manualpage = 0;
	char keybuf[6];
	init(port);
	manual(0);
	do
	{
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break;
		
		static SwkbdState swkbd;
		SwkbdButton button = SWKBD_BUTTON_NONE;
		bool finishkbd = false;
		if (kDown & KEY_X)
		{
			finishkbd = true;
			swkbdInit(&swkbd, SWKBD_TYPE_NUMPAD, 1, 5);
			itoa(port,keybuf,10);
			swkbdSetInitialText(&swkbd, keybuf);
			swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
			swkbdSetFeatures(&swkbd, SWKBD_FIXED_WIDTH);
			button = swkbdInputText(&swkbd, keybuf, sizeof(keybuf));
		}
		if (finishkbd)
		{
			if (button != SWKBD_BUTTON_NONE)
			{
				if(atoi(keybuf) > 0 && atoi(keybuf) < 65566) 
				{
					port = atoi(keybuf);
					destroy();
					init(port);
					manual(manualpage);
				} else printTop("Port needs to be a value between 1-65565\n");
			} else
				printTop("swkbd event: %d\n", swkbdGetResult(&swkbd));
		}
		
		if (kDown & KEY_L) {
			manualpage--;
			if (manualpage < 0) manualpage = 7;
			manual(manualpage);
		}
		if (kDown & KEY_R) {
			manualpage++;
			if (manualpage > 7) manualpage = 0;
			manual(manualpage);
		}
	}
	while (aptMainLoop() && loop());
	destroy();
	return 0;
}
