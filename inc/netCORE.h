/*
 * Filename: netCORE.h
 * Author:   AllAcacia
 * 
 * MODULE INFO:
 * Handles inter-console
 * connectivity.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <3ds.h>

#include "nccs_protocol.h"


/* -------- Functions -------- */

int NetCORE_Init(u64 app_id, char* usr_str_utf8, u8 max_clients);

int NetCORE_Exit(void);

int NetCORE_Execute(void);

int NetCORE_CalculateWlanCommID(void);

u32 NetCORE_GetWlanCommID(void);

bool NetCORE_GetIsInNetwork(void);

u32 NetCORE_GetConnNetworkID(void);

u16 NetCORE_GetConnNodeID(void);

void NetCORE_Print_ConnStatus(void);