/*
 * Filename: gamestate.h
 * Author:   AllAcacia
 * 
 * MODULE INFO:
 * Contains important definitions.
 */

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <3ds.h>
#include <citro2d.h>
#include <limits.h>
#include <stdlib.h>
#include <locale.h>

#define REFRESH_RATE_HZ 30
#define GAME_RATE_HZ 60
#define SBLOL_ANIM_RATE_HZ 30
#define GAME_ANIM_RATE_HZ 12 // Have to check
#define MENU_SELECT KEY_SELECT
#define PROTO_SELECT KEY_A
#define PYRO_SELECT KEY_B
#define HYDRO_SELECT KEY_Y
#define CRYO_SELECT KEY_X
#define DEBUG_SELECT KEY_ZR
#define TOP_SCREEN_WIDTH 400
#define TOP_SCREEN_HEIGHT 240
#define BOTTOM_SCREEN_WIDTH 320
#define BOTTOM_SCREEN_HEIGHT 240
#define MAX_SIMUL_STEPS 3 // prevents overloading the simulation loop

#define CFG_USERNAME_BLKSIZE 0x1C     // Number of bytes of blk
#define CFG_USERNAME_BLKID 0x000A0000 // Blk memory region in CFG Savegame
#define USERNAME_LEN_MAX 10           // # of UTF-16 characters
#define UTF8_PER_UTF16_MAX 4          // Usually spans 1-4 bytes: 1B for ASCII, 2B for accented chars, 3B for many foreign characters, 4B for emojis and anything else.
#define UTF8_PER_UTF16_CMN 3          // Common number of bytes to represent most required characters in UTF-8.

#define ENABLE_3DSLINKIO 1 // used for when you want to initialise NetCORE, as there are conflicts between it and 3dslink


typedef enum {
	MODE_MENU=0, // Menu
	MODE_PROTO,  // CJ Base
	MODE_PYRO,   // CJ Fire
	MODE_HYDRO,  // CJ Water
	MODE_CRYO,   // CJ Snow
    MODE_EXTRA   // Easter Egg
} CJQ_GameState;


int GS_Init(void);

int GS_Exit(void);

int GS_ExtractUsernameFromCFGU(void);

void GS_Username_UTF16toUTF8(u8* usr_8, u16* usr_16);

u64 GS_GetApplicationID(void);

u8* GS_GetUsername_UTF8(void);

u16* GS_GetUsername_UTF16(void);

void GS_PrintAsHex_UTF8(u8* obj);

void GS_PrintAsHex_UTF16(u16* obj);

void GS_DelayTicks(size_t tick_delay);

void GS_IncrTimeSec(void);

u64 GS_GetTickDelay_ms(u64 delay_ms);

u64 GS_GetTickDelay_hz(u64 delay_hz);

bool GS_CheckDelayTimer(u64 tick_ref, u64 delay_ticks);

void GS_SetGameState(CJQ_GameState new_gamestate);

CJQ_GameState GS_GetGameState(void);

bool GS_GetIsDebugMode(void);

C3D_RenderTarget* GS_GetTopScreen(void);

C3D_RenderTarget* GS_GetBotScreen(void);

void GS_ResetFrameTickNet(void);

void GS_IncrFrameTickNet(u64 tick_delta);

void GS_DecrFrameTickNet(u64 tick_delta);

u64 GS_GetFrameTickNet(void);

u64 GS_GetTickFrame(void);

void GS_SetTimeSec(u64 time);

u64 GS_GetTimeSec(void);

void GS_SetFrameTickPrv(u64 tick);

u64 GS_GetFrameTickPrv(void);

u64 GS_UpdTickNow(void);

u64 GS_GetTickNow(void);

u64 GS_GetSimulTicks(void);

u64 GS_GetRefreshTicks(void);

u64 GS_GetSBLOLAnimTicks(void);

u64 GS_GetGameAnimTicks(void);

#endif // GAMESTATE_H