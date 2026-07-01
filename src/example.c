/*
 * Filename: example.c
 * Author:   AllAcacia
 */

#include "netCORE.h"
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#define BUTTON_SENDSEGMENT KEY_A
#define BUTTON_CLOSENETWORK KEY_B
#define BUTTON_SEARCHNETWORK KEY_X
#define BUTTON_CREATENETWORK KEY_Y


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
    bool state_changed = true;
    bool network_owner = false;

    // Initialize services
	gfxInitDefault();

    PrintConsole topScreen, bottomScreen;
    consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM, &bottomScreen);

    consoleSelect(&topScreen);
    printf("Opened example for NCCS!\n");

    NetCORE_Init(EXAMPLE_APPID, EXAMPLE_USR);

    // Testing checksum function
    printf("Testing checksum function:\n");
    u8 array1[20] = {0x45, 0x0, 0x0, 0x1e, 0x4, 0xd2, 0x0, 0x0, 0x40, 0x6, 0x0, 0x0, 0x12, 0x34, 0x56, 0x78, 0x98, 0x76, 0x54, 0x32};
    u16 checksum1 = 0; // should turn out to be 8372
    u8 array2[24] = {0x46, 0x0, 0x0, 0x1e, 0x16, 0x2e, 0x0, 0x0, 0x40, 0x6, 0xcc, 0x59, 0x66, 0x66, 0x44, 0x44, 0x98, 0x76, 0x54, 0x32, 0x0, 0x0, 0x0, 0x0};
    u16 checksum2 = 0; // should turn out to be 0
    NetCORE_Checksum(array1, sizeof(array1), &checksum1);
    NetCORE_Checksum(array2, sizeof(array2), &checksum2);

    while(aptMainLoop()) {
        // Scan all the inputs. This should be done once for each frame
		hidScanInput();

        // Run NetCORE loop functions
        NetCORE_Execute();

		// hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();

        switch (state) {
            case NW_UNCONNECTED:
                if (state_changed) {
                    printf("\nNot connected to a network...\n");
                    printf("Press X to search for networks\n");
                    printf("Press Y to create a network\n");
                    state_changed = false;
                }
                if (kDown & BUTTON_SEARCHNETWORK) {
                    state = NW_SEARCHING;
                    state_changed = true;
                }
                if (kDown & BUTTON_CREATENETWORK) {
                    state = NW_ISCONNECTED;
                    network_owner = true;
                    state_changed = true;
                }
                break;
            
            case NW_ISCONNECTED:
                if (state_changed) {
                    if (network_owner) printf("\nCreated to a network!\n");
                    else printf("\nConnected to a network!\n");
                    printf("Press A to broadcast segment\n");
                    if (network_owner) printf("Press B to close network\n");
                    else printf("Press B to exit network\n");
                    state_changed = false;
                }
                if (kDown & BUTTON_SENDSEGMENT) {
                    printf("Broadcasting segment\n");
                }
                if (kDown & BUTTON_CLOSENETWORK) {
                    if (network_owner) {
                        printf("Closing network...\n");
                    } else {
                        printf("Exiting network...\n");
                    }
                    network_owner = false;
                    state = NW_UNCONNECTED;
                    state_changed = true;
                }
                break;
            
            case NW_SEARCHING:
                if (state_changed) {
                    printf("\nSearching for networks...\n");
                    printf("Press B to cancel searching\n");
                    state_changed = false;
                }
                if (NetCORE_GetIsInNetwork()) {
                    printf("Connected to network %lu!\n", NetCORE_GetConnNetworkID());
                    state = NW_ISCONNECTED;
                } else if (kDown & BUTTON_CLOSENETWORK) {
                    state = NW_UNCONNECTED;
                    state_changed = true;
                }
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