/*
 * Filename: netCORE.c
 * Author:   AllAcacia
 * 
 * MODULE INFO:
 * Handles inter-console
 * connectivity.
 */


#include "netCORE.h"


static NCCS_ClientProfile* NETCORE_CLIENTS; // pointer to dynamically allocated space for peer data
static u8 NCCS_MAIN_RCVBUF[NCCS_SEGMENT_MAXSIZE]; // intermediate receiving buffer, memcpy'd into peer data buffer after

static u32 USR_WLANCOMM_ID = 0; // Unique User ID
static u32 APPLICATION_ID = 0;  // Application ID

static bool CONNECTED_TO_NETWORK = false; // are we in a network?
static NCCS_NetworkInfo NETWORK_DATA;     // NCCS network struct
static u16 NETWORK_NODE_ID = 0;           // OUR node ID in the currently connected network, init 0
static u8 SESSION_CLIENTS_MAX = 0;        // how many nodes do we want to support? Kick out any extra ones


int NetCORE_Init(u64 app_id, char* usr_str_utf8, u8 max_clients)
{
	APPLICATION_ID = (u32)(app_id & 0x00000000FFFFFFFF);

	s16 ret = NCCS_Init(0x3000, usr_str_utf8); //The sharedmem size only needs to be slightly larger than the total recv_buffer_size for all binds, with page-alignment.

	if (!NetCORE_CalculateWlanCommID()) {
		return EXIT_FAILURE;
	}

	if (max_clients > NCCS_CLIENTS_MAX) {
		return EXIT_FAILURE;
	}
	SESSION_CLIENTS_MAX = max_clients;
	NETCORE_CLIENTS = calloc(SESSION_CLIENTS_MAX, sizeof(NCCS_ClientProfile));

	printf("Initialised NetCORE\n");

	return ret;
}


int NetCORE_Exit(void)
{
	NCCS_Exit();
	printf("Exited NetCORE\n");
	return EXIT_SUCCESS;
}


int NetCORE_Execute(void)
{
	return EXIT_SUCCESS;
}


int NetCORE_CalculateWlanCommID(void)
{
	// Calculate unique WLAN comm ID
    u64 console_hash;

	// We will get a u64 hash by stirring in our "app ID" (supposedly unique for every console)
	NetCORE_GenHashConsoleUnique(APPLICATION_ID, &console_hash);

	// Take every even order bit, and squash it into a u32 Wlan ID
	for (size_t i=0; i < 32; i += 1) {
		USR_WLANCOMM_ID |= ((console_hash & (1ULL << (i*2))) >> i);
	}
	if (USR_WLANCOMM_ID == 0) {
		printf("Failed to calculate a unique WLAN-ID (%lu).\n", USR_WLANCOMM_ID);
		return EXIT_FAILURE;
	} else {
		printf("Elected a unique WLAN-ID (%lu)\n", USR_WLANCOMM_ID);
		return EXIT_SUCCESS;
	}
}


u32 NetCORE_GetWlanCommID(void)
{
    return USR_WLANCOMM_ID;
}


bool NetCORE_GetIsInNetwork(void)
{
	return CONNECTED_TO_NETWORK;
}


u32 NetCORE_GetConnNetworkID(void)
{
	return NETWORK_DATA.networkID;
}


u16 NetCORE_GetConnNodeID(void)
{
	return NETWORK_NODE_ID;
}


// void NetCORE_Print_ConnStatus(void)
// {
// 	Result ret=0;
// 	u32 pos;
// 	udsConnectionStatus constatus;

// 	//By checking the output of udsGetConnectionStatus you can check for nodes (including the current one) which just (dis)connected, etc.
// 	ret = udsGetConnectionStatus(&constatus);
// 	if(R_FAILED(ret))
// 	{
// 		printf("udsGetConnectionStatus() returned 0x%08x.\n", (unsigned int)ret);
// 	}
// 	else
// 	{
// 		printf("constatus:\nstatus=0x%x\n", (unsigned int)constatus.status);
// 		printf("1=0x%x\n", (unsigned int)constatus.unk_x4);
// 		printf("cur_NetworkNodeID=0x%x\n", (unsigned int)constatus.cur_NetworkNodeID);
// 		printf("unk_xa=0x%x\n", (unsigned int)constatus.unk_xa);
// 		for(pos=0; pos<(0x20>>2); pos++)printf("%u=0x%x ", (unsigned int)pos+3, (unsigned int)constatus.unk_xc[pos]);
// 		printf("\ntotal_nodes=0x%x\n", (unsigned int)constatus.total_nodes);
// 		printf("max_nodes=0x%x\n", (unsigned int)constatus.max_nodes);
// 		printf("node_bitmask=0x%x\n", (unsigned int)constatus.total_nodes);
// 	}
// }