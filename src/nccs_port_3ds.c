/*
 * Filename: nccs_port_3ds.c
 * Author:   AllAcacia
 * 
 * MODULE INFO:
 * Handles 3DS interfacing
 * for networking.
 */


#include <3ds.h>
#include "nccs_port.h"


s16 NCCS_Init_Port(size_t sharedmem_size, const char* username)
{
    return (s16)udsInit(sharedmem_size, username);
}


void NCCS_Exit_Port(void)
{
    udsExit();
}


s16 NetCORE_GenHashConsoleUnique(u32 appIDSalt, u64 *hash)
{
    return (s16)CFGU_GenHashConsoleUnique(appIDSalt, hash);
}