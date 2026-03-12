#include "manual.h"
#include "utils.h"
#include "httpserver.h"
#include "qrcodegen.h"
#include <3ds.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern http_server *app_data;

// RGB565 Konvertierung ist hier simpel: Weiß (0xFFFF) oder Schwarz (0x0000)
void draw_pixel(u16* fb, int x, int y, u16 color) {
    // 3DS Bottom Screen: 320x240
    // Framebuffer Memory: 240x320 (Rotated 90 deg CCW)
    
    // Bounds Check
    if (x < 0 || x >= 320 || y < 0 || y >= 240) return;

    // Berechnung für rotierten Framebuffer (Standard libctru GFX_BOTTOM)
    // Screen X (0..319) bestimmt die Zeile im Speicher (0..319)
    // Screen Y (0..239) bestimmt die Spalte im Speicher, aber invertiert (239..0)
    
    // Da fb ein u16 Pointer ist, brauchen wir keine Byte-Multiplikation.
    // Stride ist 240 (Höhe des Screens = Breite des Speichers).
    int index = x * 240 + (239 - y);
    
    fb[index] = color;
}

void draw_qr_graphic(const char *text) {
	enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;
	uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
	uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
	bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, errCorLvl,
		qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
	
	if (!ok) return;

	int size = qrcodegen_getSize(qrcode);
    int scale = 3; // Skalierung
    
    // Framebuffer holen (u16* für RGB565)
    u16 fbWidth, fbHeight;
    u16* fb = (u16*)gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, &fbWidth, &fbHeight);
    
    // Zentrierung
    int start_x = (320 - (size * scale)) / 2;
    int start_y = (240 - (size * scale)) / 2 + 20;

    if (start_y < 0) start_y = 0;

    // QR Code zeichnen
    int border = 2;
    for (int y = -border; y < size + border; y++) {
        for (int x = -border; x < size + border; x++) {
            u16 color;
            if (qrcodegen_getModule(qrcode, x, y)) {
                color = 0x0000; // Schwarz
            } else {
                color = 0xFFFF; // Weiß
            }
            
            // Skalierten Pixel zeichnen
            for (int dy = 0; dy < scale; dy++) {
                for (int dx = 0; dx < scale; dx++) {
                    draw_pixel(fb, start_x + x * scale + dx, start_y + y * scale + dy, color);
                }
            }
        }
    }
}

void manual(int page) {
	const char *manual_texts[8];
	manual_texts[0] = "Loaded handlers:\n-Default Handler\n-Favicon Handler\n-System Handler\n-SDMC Handler\n-Memory R/W Handler\n-Encryption Handler\n-Note";
	manual_texts[1] = "Default Handler\nPath: /\nDescription: Returns a hardcoded HTML response";
	manual_texts[2] = "Favicon Handler\nPath: /favicon.ico\nDescription: Returns a hardcoded SVG favicon";
	manual_texts[3] = "System Handler\nPath: /system/\nDescription: /system/exit - Exits app\n/system/reboot - Reboots system";
	manual_texts[4] = "SDMC Handler\nPath: /sdcard/\nDescription: Returns content of requested file in SD Card with content-type text/html, if file is missing returns 404. Path starts at SDMC root";
	manual_texts[5] = "Memory RW Handler\nPath: /readmem/ || /writemem/\nDescription: Reads and writes system memory.\nHonestly you're more likely to crash your 3DS. Check source code for details";
	manual_texts[6] = "Encryption Handler\nPath: /crypt/\n Description: Uses native AES Encryption/Decryption Algorithms to do CBC/CTR/CTM. Check source code for details.";
	manual_texts[7] = "Note:\n\nIf the console freezes while a client requests a page, you will be unable to switch tabs until the request is completed. During this time, any inputs for switching pages in the manual on the console, switching ports, or returning to the home menu will be unavailable. The console may also stay frozen when a page is loaded other than the hardcoded one. The console will unfreeze when the user requests the hardcoded HTML again.";
	
	clearBottom();
    
    if (page == 0) {
        char url[64];
        snprintf(url, sizeof(url), "http://%s:%d/", inet_ntoa(app_data->server_addr.sin_addr), ntohs(app_data->server_addr.sin_port));
        
        printBottom("Scan to Connect:\n");
        printBottom("%s\n\n", url);
        
        draw_qr_graphic(url);
        
        printBottom("Press L/R to change pages.\nStart to Exit.");
    } else {
        int text_index = page - 1;
        if (text_index >= 0 && text_index < 8) {
            printBottom("eManual page %i\n%s\n\nPress L for previous page or R for next page.\nPress X to change port or START to exit.", page, manual_texts[text_index]);
        }
    }
	gfxFlushBuffers();
}
