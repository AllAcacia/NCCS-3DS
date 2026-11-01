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

#include <stddef.h>
#include <stdbool.h>
#include "nccs.h"


// NCCS datatype forward-declarations
typedef struct NCCS_NodeInfo_             NCCS_NodeInfo;
typedef struct NCCS_ConnectionStatus_     NCCS_ConnectionStatus;
typedef struct NCCS_NetworkStruct_        NCCS_NetworkStruct;
typedef struct NCCS_BindContext_          NCCS_BindContext;
typedef struct NCCS_NetworkScanInfo_      NCCS_NetworkScanInfo;
typedef struct NCCS_BeaconDataReplyEntry_ NCCS_BeaconDataReplyEntry;


s16 NCCS_Init_Port(size_t sharedmem_size, const char* username);

void NCCS_Exit_Port(void);

s16 NCCS_PullPacket(const NCCS_BindContext* bindcontext, void* buf, size_t size, size_t* actual_size, u16* src_NetworkNodeID);

s16 NCCS_SendPacketTo(u16 dst_NetworkNodeID, u8 data_channel, u8 flags, const void* buf, size_t size);

s16 NCCS_GenHashConsoleUnique(u32 appIDSalt, u64 *hash);


// NodeInfo
const u8* NCCS_NodeInfo_GetUserCFG(const NCCS_NodeInfo* node);
const u16* NCCS_NodeInfo_GetUsername(const NCCS_NodeInfo* node);
u16 NCCS_NodeInfo_GetNWNID(const NCCS_NodeInfo* node); // Network Node ID
u64 NCCS_NodeInfo_GetFCSeed(const NCCS_NodeInfo* node); // Friend Code Seed

// ConnStatus
u32 NCCS_ConnStatus_GetStatus(const NCCS_ConnectionStatus* status);
u32 NCCS_ConnStatus_GetCurrNWNID(const NCCS_ConnectionStatus* status);
u8 NCCS_ConnStatus_GetTotalNodes(const NCCS_ConnectionStatus* status);
u8 NCCS_ConnStatus_GetMaxNodes(const NCCS_ConnectionStatus* status);
u8 NCCS_ConnStatus_GetNodeBitmask(const NCCS_ConnectionStatus* status);

// NWStruct
const u8* NCCS_NWStruct_GetHostMAC(const NCCS_NetworkStruct* info);
u8 NCCS_NWStruct_GetChannel(const NCCS_NetworkStruct* info);
u8 NCCS_NWStruct_GetInitializedFlag(const NCCS_NetworkStruct* info);
const u8* NCCS_NWStruct_GetOUIValue(const NCCS_NetworkStruct* info);
u8 NCCS_NWStruct_GetOUIType(const NCCS_NetworkStruct* info);
u32 NCCS_NWStruct_GetWLANCommID(const NCCS_NetworkStruct* info);
u16 NCCS_NWStruct_GetAttributes(const NCCS_NetworkStruct* info);
u32 NCCS_NWStruct_GetNWID(const NCCS_NetworkStruct* info);
u8 NCCS_NWStruct_GetTotalNodes(const NCCS_NetworkStruct* info);
u8 NCCS_NWStruct_GetMaxNodes(const NCCS_NetworkStruct* info);
u8 NCCS_NWStruct_GetAppDataSize(const NCCS_NetworkStruct* info);
const u8* NCCS_NWStruct_GetAppData(const NCCS_NetworkStruct* info);

// BindContext
u32 NCCS_BindContext_GetBindNodeID(const NCCS_BindContext* context);
bool NCCS_BindContext_GetSpectator(const NCCS_BindContext* context);

// NWScanInfo
NCCS_BeaconDataReplyEntry NCCS_NWScanInfo_GetDataReplyEntry(const NCCS_NetworkScanInfo* scan);
NCCS_NetworkStruct NCCS_NWScanInfo_GetNWStruct(const NCCS_NetworkScanInfo* scan);
NCCS_NodeInfo NCCS_NWScanInfo_GetNodeInfo(const NCCS_NetworkScanInfo* scan, u8 index);

// BeaconDRE (data-reply entry [explicitly not including the header])
u8 NCCS_BeaconDRE_GetChannel(const NCCS_BeaconDataReplyEntry* entry);
const u8* NCCS_BeaconDRE_GetMAC(const NCCS_BeaconDataReplyEntry* entry);

#endif // NCCS_PORT_H