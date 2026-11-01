/*
 * Filename: nccs_port.h
 * Author:   AllAcacia
 * 
 * MODULE INFO:
 * Handles interfacing for
 * networking via NCCS.
 */


#ifndef NCCS_PORT_H
#define NCCS_PORT_H

#include "nccs.h"

s16 NCCS_Init_Port(size_t sharedmem_size, const char* username);

void NCCS_Exit_Port(void);

s16 NetCORE_GenHashConsoleUnique(u32 appIDSalt, u64 *hash);

#endif