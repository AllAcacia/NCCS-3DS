/*
 * Filename: nccs_port_3ds.c
 * Author:   AllAcacia
 * 
 * MODULE INFO:
 * Handles 3DS interfacing
 * for networking.
 */


#include <3ds.h>
#include <string.h>
#include "nccs_port.h"

// NCCS datatype definitions
struct NCCS_NodeInfo_             {udsNodeInfo obj;};
struct NCCS_ConnectionStatus_     {udsConnectionStatus obj;};
struct NCCS_NetworkStruct_        {udsNetworkStruct obj;};
struct NCCS_BindContext_          {udsBindContext obj;};
struct NCCS_NetworkScanInfo_      {udsNetworkScanInfo obj;};
struct NCCS_BeaconDataReplyEntry_ {nwmBeaconDataReplyEntry obj;};


// 3DS-port specific function implementations
s16 NCCS_Init_Port(size_t sharedmem_size, const char* username)
{
    return (s16)udsInit(sharedmem_size, username);
}


void NCCS_Exit_Port(void)
{
    udsExit();
}


s16 NCCS_PullPacket(const NCCS_BindContext* bindcontext, void* buf, size_t size, size_t* actual_size, u16* src_NetworkNodeID)
{
    return (s16)udsPullPacket(&bindcontext->obj, buf, size, actual_size, src_NetworkNodeID);
}


s16 NCCS_SendPacketTo(u16 dst_NetworkNodeID, u8 data_channel, u8 flags, const void* buf, size_t size)
{
    return (s16)udsSendTo(dst_NetworkNodeID, data_channel, flags, buf, size);
}


s16 NCCS_GenHashConsoleUnique(u32 appIDSalt, u64 *hash)
{
    return (s16)CFGU_GenHashConsoleUnique(appIDSalt, hash);
}


// NodeInfo
const u8* NCCS_NodeInfo_GetUserCFG(const NCCS_NodeInfo* node)   {return node->obj.usercfg;}
const u16* NCCS_NodeInfo_GetUsername(const NCCS_NodeInfo* node) {return node->obj.username;}
u16 NCCS_NodeInfo_GetNWNID(const NCCS_NodeInfo* node)           {return node->obj.NetworkNodeID;}
u64 NCCS_NodeInfo_GetFCSeed(const NCCS_NodeInfo* node)          {return node->obj.uds_friendcodeseed;}

// ConnStatus
u32 NCCS_ConnStatus_GetStatus(const NCCS_ConnectionStatus* status)     {return status->obj.status;}
u32 NCCS_ConnStatus_GetCurrNWNID(const NCCS_ConnectionStatus* status)  {return status->obj.cur_NetworkNodeID;}
u8 NCCS_ConnStatus_GetTotalNodes(const NCCS_ConnectionStatus* status)  {return status->obj.total_nodes;}
u8 NCCS_ConnStatus_GetMaxNodes(const NCCS_ConnectionStatus* status)    {return status->obj.max_nodes;}
u8 NCCS_ConnStatus_GetNodeBitmask(const NCCS_ConnectionStatus* status) {return status->obj.node_bitmask;}

// NWStruct
const u8* NCCS_NWStruct_GetHostMAC(const NCCS_NetworkStruct* info)  {return info->obj.host_macaddress;}
u8 NCCS_NWStruct_GetChannel(const NCCS_NetworkStruct* info)         {return info->obj.channel;}
u8 NCCS_NWStruct_GetInitializedFlag(const NCCS_NetworkStruct* info) {return info->obj.initialized_flag;}
const u8* NCCS_NWStruct_GetOUIValue(const NCCS_NetworkStruct* info) {return info->obj.oui_value;}
u8 NCCS_NWStruct_GetOUIType(const NCCS_NetworkStruct* info)         {return info->obj.oui_type;}
u32 NCCS_NWStruct_GetWLANCommID(const NCCS_NetworkStruct* info)     {return info->obj.wlancommID;}
u16 NCCS_NWStruct_GetAttributes(const NCCS_NetworkStruct* info)     {return info->obj.attributes;}
u32 NCCS_NWStruct_GetNWID(const NCCS_NetworkStruct* info)           {return info->obj.networkID;}
u8 NCCS_NWStruct_GetTotalNodes(const NCCS_NetworkStruct* info)      {return info->obj.total_nodes;}
u8 NCCS_NWStruct_GetMaxNodes(const NCCS_NetworkStruct* info)        {return info->obj.max_nodes;}
u8 NCCS_NWStruct_GetAppDataSize(const NCCS_NetworkStruct* info)     {return info->obj.appdata_size;}
const u8* NCCS_NWStruct_GetAppData(const NCCS_NetworkStruct* info)  {return info->obj.appdata;}

// BindContext
u32 NCCS_BindContext_GetBindNodeID(const NCCS_BindContext* context) {return context->obj.BindNodeID;}
bool NCCS_BindContext_GetSpectator(const NCCS_BindContext* context) {return context->obj.spectator;}

// NWScanInfo
NCCS_BeaconDataReplyEntry NCCS_NWScanInfo_GetDataReplyEntry(const NCCS_NetworkScanInfo* scan) {NCCS_BeaconDataReplyEntry ret = {.obj = scan->obj.datareply_entry}; return ret;}
NCCS_NetworkStruct NCCS_NWScanInfo_GetNWStruct(const NCCS_NetworkScanInfo* scan)              {NCCS_NetworkStruct ret = {.obj = scan->obj.network}; return ret;}
NCCS_NodeInfo NCCS_NWScanInfo_GetNodeInfo(const NCCS_NetworkScanInfo* scan, u8 index)
{
    NCCS_NodeInfo ret = {0};
    if (scan == NULL || index >= scan->obj.network.total_nodes) {
        return ret;
    }
    ret.obj = scan->obj.nodes[index];
    return ret;
}

// BeaconDRE (data-reply entry [explicitly not including the header])
u8 NCCS_BeaconDRE_GetChannel(const NCCS_BeaconDataReplyEntry* entry)    {return entry->obj.channel;}
const u8* NCCS_BeaconDRE_GetMAC(const NCCS_BeaconDataReplyEntry* entry) {return entry->obj.mac_address;}