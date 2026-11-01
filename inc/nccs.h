/*
 * Filename: nccs.h
 * Author:   AllAcacia
 * 
 * MODULE INFO:
 * Key NCCS macro and types.
 */


#ifndef NCCS_H
#define NCCS_H

#include <stdint.h>

/* -------- Constant NCCS Macros -------- */
#define NCCS_NWNODES_MAX 16 // UDS supports 16 total "nodes" in a network.
#define NCCS_CLIENTS_MAX NCCS_NWNODES_MAX - 1 // exclude yourself
#define NCCS_APPDATA_MAX 0xC8
#define NCCS_SEGMENT_MAXSIZE 0x5C6
#define NCCS_WINDOW_MAXSIZE 16
#define NCCS_USRCHAR_MAXSIZE 11 // including terminating char

/* ------- Typedef NCCS Datatypes ------- */
typedef int8_t s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;

#endif // NCCS_H