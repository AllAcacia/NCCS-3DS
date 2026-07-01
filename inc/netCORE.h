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
#define NETCORE_BYTES_PER_WORD 4

int NetCORE_Init(u64 app_id, char* usr_str_utf8);

int NetCORE_Exit(void);

int NetCORE_CalculateWlanCommID(void);

u32 NetCORE_GetWlanCommID(void);

int NetCORE_Checksum(const void* array, const size_t size, u16* checksum);

bool NetCORE_GetIsInNetwork(void);

u32 NetCORE_GetConnNetworkID(void);

void NetCORE_Print_ConnStatus(void);

/* ---------------- example from 3ds-examples ---------------- */
void NetCORE_UDS_Test(void);

int NetCORE_Example(void);