/*
 * Filename: nccs_port_win.c
 * Author:   AllAcacia
 * 
 * MODULE INFO:
 * Handles Windows interfacing
 * for networking.
 */


#include "nccs_port.h"
#include <stdlib.h>
#include <time.h>


s16 NCCS_Init_Port(size_t sharedmem_size, const char* username)
{
    return EXIT_SUCCESS;
}


void NCCS_Exit_Port(void)
{
    return;
}


u32 NCCS_NWStruct_GetNWID(const NCCS_NetworkStruct* info)
{
    return 0;
}


s16 NCCS_GenHashConsoleUnique(u32 appIDSalt, u64 *hash)
{
    if (hash == NULL)
        return EXIT_FAILURE;

    struct timespec ts;

    if (timespec_get(&ts, TIME_UTC) != TIME_UTC)
        return EXIT_FAILURE;

    *hash = ((u64)ts.tv_sec << 32)
          ^ (u64)ts.tv_nsec
          ^ (u64)appIDSalt;

    return EXIT_SUCCESS;
}