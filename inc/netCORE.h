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

#define NETCORE_BUFFER_SIZE UDS_DATAFRAME_MAXSIZE

int NetCORE_Init(u64 app_id, char* usr_str_utf8);

int NetCORE_Exit(void);

int NetCORE_CalculateWlanCommID(void);

u32 NetCORE_GetWlanCommID(void);

void NetCORE_Print_ConnStatus(void);

void NetCORE_UDS_Test(void);

int NetCORE_Example(void);