/*
 * Filename: example.c
 * Author:   AllAcacia
 */

#include "netCORE.h"
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>


typedef enum {
    NW_UNCONNECTED=0,
    NW_ISCONNECTED,
    NW_SEARCHING
} NW_State;


int main(void)
{
    char* EXAMPLE_USR = "AllAcacia";
    u64 EXAMPLE_APPID = 0x000000000ACA6400;

    NW_State state = NW_UNCONNECTED;

    // Initialize services
	gfxInitDefault();

    PrintConsole topScreen, bottomScreen;
    consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM, &bottomScreen);

    consoleSelect(&topScreen);
    printf("Opened example for NCCS!\n");

    NetCORE_Init(EXAMPLE_APPID, EXAMPLE_USR);

    while(aptMainLoop()) {
        //Scan all the inputs. This should be done once for each frame
		hidScanInput();

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();

        switch (state) {
            case NW_UNCONNECTED:
                break;
            
            case NW_ISCONNECTED:
                break;
            
            case NW_SEARCHING:
                break;
            
            default:
                break;
        }
        
        if (kDown & KEY_START) break; // break in order to return to hbmenu

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();

		//Wait for VBlank
		gspWaitForVBlank();
    }

    // Exit services
	gfxExit();

	return EXIT_SUCCESS;
}