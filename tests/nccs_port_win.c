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


s16 NetCORE_GenHashConsoleUnique(u32 appIDSalt, u64 *hash)
{
    time_t timestamp;
    time(&timestamp);
    *hash = timestamp;
    
    return EXIT_SUCCESS;
}