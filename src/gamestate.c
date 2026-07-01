/*
 * Filename: gamestate.c
 * Author:   AllAcacia
 * 
 * MODULE INFO:
 * Contains important definitions.
 */


#include "gamestate.h"

static u64 tick_now = 0;

static u64 frame_tick_prv = 0;
static u64 frame_tick_net = 0;

static u64 timer_tick_prv;
static u64 time_sec = 0;

static u64 SIMUL_TICKS = 0;
static u64 REFRESH_TICKS = 0;
static u64 SBLOL_ANIM_TICKS = 0;
static u64 GAME_ANIM_TICKS = 0;

static bool debug_mode = false;

static CJQ_GameState gamestate;
static C3D_RenderTarget* TOP_SCREEN;
static C3D_RenderTarget* BOT_SCREEN;

static const u64 CJQ_APPLICATION_ID = 0x000400000ACA6400;                   // Ideally this is the appID I want :3
static u16 CJQ_USERNAME_UTF16[USERNAME_LEN_MAX+1] = {0};                    // Username from system (utf-16)
static u8 CJQ_USERNAME_UTF8[(UTF8_PER_UTF16_CMN*USERNAME_LEN_MAX)+1] = {0}; // Username from system (utf-8)


int GS_Init(void)
{
    // Initialize CFGU
    cfguInit();

    // Initialize screens
	GS_SetTimeSec(0);
	GS_SetGameState(MODE_MENU);
	
	// Set tick tracking states
    GS_UpdTickNow();
	GS_SetFrameTickPrv(GS_GetTickNow());
	SIMUL_TICKS = GS_GetTickDelay_hz(GAME_RATE_HZ);
	REFRESH_TICKS = GS_GetTickDelay_hz(REFRESH_RATE_HZ);
    SBLOL_ANIM_TICKS = GS_GetTickDelay_hz(SBLOL_ANIM_RATE_HZ);
    GAME_ANIM_TICKS = GS_GetTickDelay_hz(GAME_ANIM_RATE_HZ);
    GS_ResetFrameTickNet();

    // Create screens
    TOP_SCREEN = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	BOT_SCREEN = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    // Extract username from CFG Savegame
    if (!GS_ExtractUsernameFromCFGU()) {
        return EXIT_FAILURE;
    }

    printf("Init key gamestate variables\n");

    return EXIT_SUCCESS;
}


int GS_Exit(void)
{
    cfguExit();
    return EXIT_SUCCESS;
}


int GS_ExtractUsernameFromCFGU(void)
{
    // Retrieve username from CFG savegame
    u8 cjq_username_blk[CFG_USERNAME_BLKSIZE] = {0}; // Username blk from system
    Result res = CFGU_GetConfigInfoBlk2(CFG_USERNAME_BLKSIZE, CFG_USERNAME_BLKID, cjq_username_blk);
    if (R_FAILED(res)) {
        printf("Failed to load CFG blk\n");
        printf("summ =  %ld\n", R_SUMMARY(res));
        printf("level = %ld\n", R_LEVEL(res));
        printf("desc =  %ld\n", R_DESCRIPTION(res));
        return EXIT_FAILURE;
    }
    
    // Extract relevant bytes from the data blk
    // utf-16
    memcpy(CJQ_USERNAME_UTF16, (u16*)cjq_username_blk, USERNAME_LEN_MAX*sizeof(CJQ_USERNAME_UTF16[0]));
    // utf-8
    GS_Username_UTF16toUTF8(CJQ_USERNAME_UTF8, CJQ_USERNAME_UTF16);

    return EXIT_SUCCESS;
}


void GS_Username_UTF16toUTF8(u8* usr_8, u16* usr_16)
{
    ssize_t ret = utf16_to_utf8(usr_8, usr_16, UTF8_PER_UTF16_CMN*USERNAME_LEN_MAX*sizeof(u8));

    printf("Username (UTF-16): ");
    GS_PrintAsHex_UTF16(CJQ_USERNAME_UTF16);
    printf("Username  (UTF-8): ");
    GS_PrintAsHex_UTF8(CJQ_USERNAME_UTF8);

    if (ret < 0) {
        printf("Error when converting utf16 to utf8\n");
    } else {
        printf("Converted UTF-16 characters for username to (%d) UTF-8 characters\n", ret);
    }
}


u64 GS_GetApplicationID(void)
{
    return CJQ_APPLICATION_ID;
}


u8* GS_GetUsername_UTF8(void)
{
    return CJQ_USERNAME_UTF8;
}


u16* GS_GetUsername_UTF16(void)
{
    return CJQ_USERNAME_UTF16;
}


void GS_PrintAsHex_UTF8(u8* obj)
{
    for (int i = 0; obj[i] != 0; i++) {
        printf("%02X ", (unsigned char)obj[i]);
    }
    printf("\n");
}


void GS_PrintAsHex_UTF16(u16* obj)
{
    for (int i = 0; obj[i] != 0; i++) {
        u16 seg = obj[i];
        printf("%02X%02X ", (seg >> 8) & 0xFF, seg & 0xFF);
    }
    printf("\n");
}


void GS_DelayTicks(size_t tick_delay)
{
    GS_SetFrameTickPrv(GS_GetTickNow());
    while(!GS_CheckDelayTimer(frame_tick_prv, tick_delay)) {
        GS_UpdTickNow();
    }
}


void GS_IncrTimeSec(void)
{
    if (GS_CheckDelayTimer(timer_tick_prv, GS_GetTickDelay_ms(1000))) {
        time_sec += 1;
        timer_tick_prv = svcGetSystemTick();
    }
}


u64 GS_GetTickDelay_ms(u64 delay_ms)
{
	u64 delay_ticks = (delay_ms * SYSCLOCK_ARM11) / 1000;
	return delay_ticks;
}


u64 GS_GetTickDelay_hz(u64 delay_hz)
{
	u64 delay_ticks = SYSCLOCK_ARM11 / delay_hz;
	return delay_ticks;
}


bool GS_CheckDelayTimer(u64 tick_ref, u64 delay_ticks)
{
	bool result;
    u64 delta = 0;
	if  (tick_now >= tick_ref) {
        delta = tick_now - tick_ref;
		result = delta > delay_ticks;
	} else {
        delta = tick_now + (ULLONG_MAX - tick_ref);
		result = delta > delay_ticks;
	}

	return result;
}


void GS_SetGameState(CJQ_GameState new_gamestate)
{
    gamestate = new_gamestate;
}


CJQ_GameState GS_GetGameState(void)
{
    return gamestate;
}


bool GS_GetIsDebugMode(void)
{
    return debug_mode;
}


C3D_RenderTarget* GS_GetTopScreen(void)
{
    return TOP_SCREEN;
}


C3D_RenderTarget* GS_GetBotScreen(void)
{
    return BOT_SCREEN;
}


void GS_ResetFrameTickNet(void)
{
    frame_tick_net = 0;
}


void GS_IncrFrameTickNet(u64 tick_delta)
{
    frame_tick_net += tick_delta;
}


void GS_DecrFrameTickNet(u64 tick_delta)
{
    frame_tick_net -= tick_delta;
}


u64 GS_GetFrameTickNet(void)
{
    return frame_tick_net;
}


u64 GS_GetTickFrame(void)
{
    if (GS_GetTickNow() >= GS_GetFrameTickPrv()) {
        return GS_GetTickNow() - GS_GetFrameTickPrv();
    } else {
        return GS_GetTickNow() + (ULLONG_MAX - GS_GetFrameTickPrv());
    }
}


void GS_SetTimeSec(u64 time)
{
    time_sec = time;
}


u64 GS_GetTimeSec(void)
{
    return time_sec;
}


void GS_SetFrameTickPrv(u64 tick)
{
    frame_tick_prv = tick;
}


u64 GS_GetFrameTickPrv(void)
{
    return frame_tick_prv;
}


u64 GS_UpdTickNow(void)
{
    tick_now = svcGetSystemTick();
    return tick_now;
}


u64 GS_GetTickNow(void)
{
    return tick_now;
}


u64 GS_GetSimulTicks(void)
{
    return SIMUL_TICKS;
}


u64 GS_GetRefreshTicks(void)
{
    return REFRESH_TICKS;
}


u64 GS_GetSBLOLAnimTicks(void)
{
    return SBLOL_ANIM_TICKS;
}


u64 GS_GetGameAnimTicks(void)
{
    return GAME_ANIM_TICKS;
}