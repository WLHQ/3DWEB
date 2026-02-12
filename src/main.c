#include "httpserver.h"
#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include "manual.h"

SystemConfig sys_conf;
int	main(int ac, char **av)
{
    // --- SMART DETECTION START ---
    APT_CheckNew3DS(&sys_conf.is_new_3ds);

    if (sys_conf.is_new_3ds) {
        // NEW 3DS: Gib ihm alles was wir haben!
        osSetSpeedupEnable(true); // 804 MHz Modus
        sys_conf.stack_size = 64 * 1024;      // 64KB Stack für Threads
        sys_conf.socket_buffer_size = 32 * 1024; // 32KB TCP Buffer
    } else {
        // OLD 3DS: Ressourcen sparen
        osSetSpeedupEnable(false);
        sys_conf.stack_size = 16 * 1024;      // 16KB Stack (reicht für HTTP Parsing)
        sys_conf.socket_buffer_size = 4 * 1024;  // 4KB TCP Buffer (Standard)
    }
    // --- SMART DETECTION ENDE ---
	
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
