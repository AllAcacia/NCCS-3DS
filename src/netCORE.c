/*
 * Filename: netCORE.c
 * Author:   AllAcacia
 * 
 * MODULE INFO:
 * Handles inter-console
 * connectivity.
 */

#include "netCORE.h"

static NCCS_NetworkPeer* NETCORE_CLIENTS; // pointer to dynamically allocated space for peer data
static u8 NCCS_MAIN_RCVBUF[NCCS_MAIN_RCVBUF_BUFSIZE_MAX]; // intermediate receiving buffer, memcpy'd into peer data buffer after

static u32 USR_WLANCOMM_ID = 0; // Unique User ID
static u32 APPLICATION_ID = 0;  // Application ID

static bool CONNECTED_TO_NETWORK = false; // are we in a network?
static udsNetworkStruct NETWORK_DATA;     // UDS network struct
static u16 NETWORK_NODE_ID = 0;           // OUR node ID in the currently connected network, init 0
static u8 SESSION_CLIENTS_MAX = 0;        // how many nodes do we want to support? Kick out any extra ones


int NetCORE_Init(u64 app_id, char* usr_str_utf8, u8 max_clients)
{
	APPLICATION_ID = (u32)(app_id & 0x00000000FFFFFFFF);

	#ifndef ENABLE_3DSLINKIO // 3dslink and udsInit have conflicts from testing
	Result ret=0;
	ret = udsInit(0x3000, usr_str_utf8); //The sharedmem size only needs to be slightly larger than the total recv_buffer_size for all binds, with page-alignment.
	if (R_FAILED(ret)) {
		printf("udsInit failed: 0x%08x.\n", (unsigned int)ret);
		return EXIT_FAILURE;
	}
	#endif

	if (!NetCORE_CalculateWlanCommID()) {
		return EXIT_FAILURE;
	}

	if (max_clients > NCCS_CLIENTS_MAX) {
		return EXIT_FAILURE;
	}
	SESSION_CLIENTS_MAX = max_clients;
	NETCORE_CLIENTS = calloc(SESSION_CLIENTS_MAX, sizeof(NCCS_NetworkPeer));

	printf("Initialised NetCORE\n");

	return EXIT_SUCCESS;
}


int NetCORE_Exit(void)
{
	udsExit();
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
	CFGU_GenHashConsoleUnique(APPLICATION_ID, &console_hash);

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


s16 NCCS_CalcWindowIndex(u32 query_seqno, u32 base_seqno, u8 base_index, u8 window_size)
{
	// WARNING: does not handle seqnos that have looped from integer overflow

	if (window_size == 0) return -3; // cannot divide by 0
	if (query_seqno < base_seqno) return -2; // duplicate segment, send ACK
	if ((base_seqno + window_size) <= query_seqno) return -1; // reject sequence number

	return (s16)((base_index + (query_seqno - base_seqno)) % window_size);
}


s16 NCCS_CalcRxWindowIndex(NCCS_RxSlidingWindow* window, u32 query_seqno)
{
	return NCCS_CalcWindowIndex(query_seqno, window->rx_base_seqno, window->rx_base_index, window->rx_size);
}


s16 NCCS_CalcTxWindowIndex(NCCS_TxSlidingWindow* window, u32 query_seqno)
{
	return NCCS_CalcWindowIndex(query_seqno, window->tx_base_seqno, window->tx_base_index, window->tx_size);
}


int NCCS_CalcChecksum(const void* segment, size_t seg_size, u16* checksum)
{
	// Case: segment size in memory is less than advertised
	if (NCCS_HDR_LEN + NCCS_GetPayloadLen(segment) > seg_size) {
		printf("Checksum: decl. size is more than actual\n");
    	return EXIT_FAILURE;
	}
	// Case: segment size is larger than allowed through UDS
	if (seg_size > UDS_DATAFRAME_MAXSIZE) {
		printf("Checksum: segment exceeds MTU (%zu>%zu)\n", seg_size, UDS_DATAFRAME_MAXSIZE);
		return EXIT_FAILURE;
	}
	// Case: segment not multiple of 2
	if (seg_size % 2 != 0) {
		printf("Checksum: size not multiple of 2\n");
		return EXIT_FAILURE;
	}
	// Case: NULL pointer
	if (checksum == NULL) {
		printf("Checksum: NULL pointer passed");
		return EXIT_FAILURE;
	}

	// Calculate checksum
	u64 temp = 0;
	for (size_t i=0; i < (NCCS_HDR_LEN-2); i+=2) {    // ignore final 2 bytes (checksum location)
		temp += NCCS_GetBE16b(segment, i);
	}
	for (size_t i=NCCS_HDR_LEN; i < seg_size; i+=2) { // process payload bytes
		temp += NCCS_GetBE16b(segment, i);
	}

	while (temp > UINT16_MAX) {
		temp = (temp & 0xFFFF) + (temp >> 16);
	}

	temp = temp ^ 0xFFFF;
	printf("Checksum: %llu\n", temp);
	*checksum = temp;

	return EXIT_SUCCESS;
}


u8 NCCS_GetBE1b(const void* segment, size_t byte_offset, u8 bit_n)
{
	return (((u8*)segment)[byte_offset] >> bit_n) & 0x01;
}


int NCCS_SetBE1b(void* segment, size_t byte_offset, u8 bit_n, u8 val)
{
	if (val <= 1) {
		((u8*)segment)[byte_offset] &= ~(1 << bit_n);  // clear bit
		((u8*)segment)[byte_offset] |= (val << bit_n); // rewrite bit
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}


u16 NCCS_GetBE16b(const void* segment, size_t offset)
{
	const u8* bytes = segment;
	return (bytes[offset] << 8) | bytes[offset+1];
}


void NCCS_SetBE16b(void* segment, size_t offset, u16 val)
{
	((u8*)segment)[offset] = (val >> 8) & 0x00FF;
	((u8*)segment)[offset+1] = val & 0x00FF;
}


u32 NCCS_GetBE32b(const void* segment, size_t offset)
{
	const u8* bytes = segment;
	return (bytes[offset] << 24) | (bytes[offset+1] << 16) | (bytes[offset+2] << 8) | bytes[offset+3];
}


void NCCS_SetBE32b(void* segment, size_t offset, u32 val)
{
	((u8*)segment)[offset] = (val >> 24) & 0x000000FF;
	((u8*)segment)[offset+1] = (val >> 16) & 0x000000FF;
	((u8*)segment)[offset+2] = (val >> 8) & 0x000000FF;
	((u8*)segment)[offset+3] = val & 0x000000FF;
}


u32 NCCS_GetAppID(const void* segment)
{
	return NCCS_GetBE32b(segment, NCCS_APPID_BYTE_OFFSET);
}


void NCCS_SetAppID(void* segment, u32 app_id)
{
	NCCS_SetBE32b(segment, NCCS_APPID_BYTE_OFFSET, app_id);
}


u16 NCCS_GetSrceNWNID(const void* segment)
{
	return NCCS_GetBE16b(segment, NCCS_SRCENWNID_BYTE_OFFSET);
}


void NCCS_SetSrceNWNID(void* segment, u16 srce_nwnid)
{
	NCCS_SetBE16b(segment, NCCS_SRCENWNID_BYTE_OFFSET, srce_nwnid);
}


u16 NCCS_GetDestNWNID(const void* segment)
{
	return NCCS_GetBE16b(segment, NCCS_DESTNWNID_BYTE_OFFSET);
}


void NCCS_SetDestNWNID(void* segment, u16 dest_nwnid)
{
	NCCS_SetBE16b(segment, NCCS_DESTNWNID_BYTE_OFFSET, dest_nwnid);
}


u32 NCCS_GetSrceWLANID(const void* segment)
{
	return NCCS_GetBE32b(segment, NCCS_SRCEWLANID_BYTE_OFFSET);
}


void NCCS_SetSrceWLANID(void* segment, u32 srce_wlanid)
{
	NCCS_SetBE32b(segment, NCCS_SRCEWLANID_BYTE_OFFSET, srce_wlanid);
}


u32 NCCS_GetDestWLANID(const void* segment)
{
	return NCCS_GetBE32b(segment, NCCS_DESTWLANID_BYTE_OFFSET);
}


void NCCS_SetDestWLANID(void* segment, u32 dest_wlanid)
{
	NCCS_SetBE32b(segment, NCCS_DESTWLANID_BYTE_OFFSET, dest_wlanid);
}


u32 NCCS_GetSeqno(const void* segment)
{
	return NCCS_GetBE32b(segment, NCCS_SEQNO_BYTE_OFFSET);
}


void NCCS_SetSeqno(void* segment, u32 seqno)
{
	NCCS_SetBE32b(segment, NCCS_SEQNO_BYTE_OFFSET, seqno);
}


u32 NCCS_GetAckno(const void* segment)
{
	return NCCS_GetBE32b(segment, NCCS_ACKNO_BYTE_OFFSET);
}


void NCCS_SetAckno(void* segment, u32 ackno)
{
	NCCS_SetBE32b(segment, NCCS_ACKNO_BYTE_OFFSET, ackno);
}


u16 NCCS_GetAppMsgType(const void* segment)
{
	return NCCS_GetBE16b(segment, NCCS_APPMSGTYPE_BYTE_OFFSET);
}


void NCCS_SetAppMsgType(void* segment, u16 appmsgtype)
{
	NCCS_SetBE16b(segment, NCCS_APPMSGTYPE_BYTE_OFFSET, appmsgtype);
}


u8 NCCS_GetSynFlag(const void* segment)
{
	return NCCS_GetBE1b(segment, NCCS_SYNFLAG_BYTE_OFFSET, NCCS_SYNFLAG_BIT_N);
}


void NCCS_SetSynFlag(void* segment, u8 syn)
{
	NCCS_SetBE1b(segment, NCCS_SYNFLAG_BYTE_OFFSET, NCCS_SYNFLAG_BIT_N, syn);
}


u8 NCCS_GetAckFlag(const void* segment)
{
	return NCCS_GetBE1b(segment, NCCS_ACKFLAG_BYTE_OFFSET, NCCS_ACKFLAG_BIT_N);
}


void NCCS_SetAckFlag(void* segment, u8 ack)
{
	NCCS_SetBE1b(segment, NCCS_ACKFLAG_BYTE_OFFSET, NCCS_ACKFLAG_BIT_N, ack);
}


u8 NCCS_GetNakFlag(const void* segment)
{
	return NCCS_GetBE1b(segment, NCCS_NAKFLAG_BYTE_OFFSET, NCCS_NAKFLAG_BIT_N);
}


void NCCS_SetNakFlag(void* segment, u8 nak)
{
	NCCS_SetBE1b(segment, NCCS_NAKFLAG_BYTE_OFFSET, NCCS_NAKFLAG_BIT_N, nak);
}


u8 NCCS_GetFinFlag(const void* segment)
{
	return NCCS_GetBE1b(segment, NCCS_FINFLAG_BYTE_OFFSET, NCCS_FINFLAG_BIT_N);
}


void NCCS_SetFinFlag(void* segment, u8 fin)
{
	NCCS_SetBE1b(segment, NCCS_FINFLAG_BYTE_OFFSET, NCCS_FINFLAG_BIT_N, fin);
}


u16 NCCS_GetPayloadLen(const void* segment)
{
	return NCCS_GetBE16b(segment, NCCS_PAYLOADLEN_BYTE_OFFSET);
}


void NCCS_SetPayloadLen(void* segment, u16 payload_len)
{
	NCCS_SetBE16b(segment, NCCS_PAYLOADLEN_BYTE_OFFSET, payload_len);
}


u16 NCCS_GetChecksum(const void* segment)
{
	return NCCS_GetBE16b(segment, NCCS_CHECKSUM_BYTE_OFFSET);
}


void NCCS_SetChecksum(void* segment, u16 checksum)
{
	NCCS_SetBE16b(segment, NCCS_CHECKSUM_BYTE_OFFSET, checksum);
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


void NetCORE_Print_ConnStatus(void)
{
	Result ret=0;
	u32 pos;
	udsConnectionStatus constatus;

	//By checking the output of udsGetConnectionStatus you can check for nodes (including the current one) which just (dis)connected, etc.
	ret = udsGetConnectionStatus(&constatus);
	if(R_FAILED(ret))
	{
		printf("udsGetConnectionStatus() returned 0x%08x.\n", (unsigned int)ret);
	}
	else
	{
		printf("constatus:\nstatus=0x%x\n", (unsigned int)constatus.status);
		printf("1=0x%x\n", (unsigned int)constatus.unk_x4);
		printf("cur_NetworkNodeID=0x%x\n", (unsigned int)constatus.cur_NetworkNodeID);
		printf("unk_xa=0x%x\n", (unsigned int)constatus.unk_xa);
		for(pos=0; pos<(0x20>>2); pos++)printf("%u=0x%x ", (unsigned int)pos+3, (unsigned int)constatus.unk_xc[pos]);
		printf("\ntotal_nodes=0x%x\n", (unsigned int)constatus.total_nodes);
		printf("max_nodes=0x%x\n", (unsigned int)constatus.max_nodes);
		printf("node_bitmask=0x%x\n", (unsigned int)constatus.total_nodes);
	}
}

/* ---------------- example from 3ds-examples ---------------- */
void NetCORE_UDS_Test(void)
{
	Result ret=0;
	u32 con_type=0;

	u32 *tmpbuf;
	size_t tmpbuf_size;

	u8 data_channel = 1;
	udsNetworkStruct networkstruct;
	udsBindContext bindctx;
	udsNetworkScanInfo* networks = NULL;
	udsNetworkScanInfo* network = NULL;
	size_t total_networks = 0;

	u32 recv_buffer_size = UDS_DEFAULT_RECVBUFSIZE;
	char* passphrase = "udsdemo passphrase c186093cd2652741";//Change this passphrase to your own. The input you use for the passphrase doesn't matter since it's a raw buffer.

	udsConnectionType conntype = UDSCONTYPE_Client;

	u32 transfer_data, prev_transfer_data = 0;
	size_t actual_size;
	u16 src_NetworkNodeID;
	u32 tmp=0;
	u32 pos;

	udsNodeInfo tmpnode;

	u8 appdata[0x14] = {0x69, 0x8a, 0x05, 0x5c};
	u8 out_appdata[0x14];

	char tmpstr[256];

	strncpy((char*)&appdata[4], "Test appdata.", sizeof(appdata)-4);

	printf("Successfully initialized.\n");

	tmpbuf_size = 0x4000;
	tmpbuf = malloc(tmpbuf_size);
	if(tmpbuf==NULL)
	{
		printf("Failed to allocate tmpbuf for beacon data.\n");
		return;
	}

	//With normal client-side handling you'd keep running network-scanning until the user chooses to stops scanning or selects a network to connect to. This example just scans a maximum of 10 times until at least one network is found.
	for(pos=0; pos<10; pos++)
	{
		total_networks = 0;
		memset(tmpbuf, 0, sizeof(tmpbuf_size));
		ret = udsScanBeacons(tmpbuf, tmpbuf_size, &networks, &total_networks, USR_WLANCOMM_ID, 0, NULL, false);
		printf("udsScanBeacons() returned 0x%08x.\ntotal_networks=%u.\n", (unsigned int)ret, (unsigned int)total_networks);

		if(total_networks)break;
	}

	free(tmpbuf);
	tmpbuf = NULL;

	if(total_networks)
	{
		//At this point you'd let the user select which network to connect to and optionally display the first node's username(the host), along with the parsed appdata if you want. For this example this just uses the first detected network and then displays the username of each node.
		//If appdata isn't enough, you can do what DLP does loading the icon data etc: connect to the network as a spectator temporarily for receiving broadcasted data frames.

		network = &networks[0];

		printf("network: total nodes = %u.\n", (unsigned int)network->network.total_nodes);

		for(pos=0; pos<UDS_MAXNODES; pos++)
		{
			if(!udsCheckNodeInfoInitialized(&network->nodes[pos]))continue;

			memset(tmpstr, 0, sizeof(tmpstr));

			ret = udsGetNodeInfoUsername(&network->nodes[pos], tmpstr);
			if(R_FAILED(ret))
			{
				printf("udsGetNodeInfoUsername() returned 0x%08x.\n", (unsigned int)ret);
				free(networks);
				return;
			}

			printf("node%u username: %s\n", (unsigned int)pos, tmpstr);
		}

		//You can load appdata from the scanned beacon data here if you want.
		actual_size = 0;
		ret = udsGetNetworkStructApplicationData(&network->network, out_appdata, sizeof(out_appdata), &actual_size);
		if(R_FAILED(ret) || actual_size!=sizeof(out_appdata))
		{
			printf("udsGetNetworkStructApplicationData() returned 0x%08x. actual_size = 0x%x.\n", (unsigned int)ret, actual_size);
			free(networks);
			return;
		}

		memset(tmpstr, 0, sizeof(tmpstr));
		if(memcmp(out_appdata, appdata, 4)!=0)
		{
			printf("The first 4-bytes of appdata is invalid.\n");
			free(networks);
			return;
		}

		strncpy(tmpstr, (char*)&out_appdata[4], sizeof(out_appdata)-5);
		tmpstr[sizeof(out_appdata)-6]='\0';

		printf("String from network appdata: %s\n", (char*)&out_appdata[4]);

		hidScanInput();//Normally you would only connect as a regular client.
		if(hidKeysHeld() & KEY_L)
		{
			conntype = UDSCONTYPE_Spectator;
			printf("Connecting to the network as a spectator...\n");
		}
		else
		{
			printf("Connecting to the network as a client...\n");
		}

		for(pos=0; pos<10; pos++)
		{
			ret = udsConnectNetwork(&network->network, passphrase, strlen(passphrase)+1, &bindctx, UDS_BROADCAST_NETWORKNODEID, conntype, data_channel, recv_buffer_size);
			if(R_FAILED(ret))
			{
				printf("udsConnectNetwork() returned 0x%08x.\n", (unsigned int)ret);
			}
			else
			{
				break;
			}
		}

		free(networks);

		if(pos==10)return;

		printf("Connected.\n");

		tmp = 0;
		ret = udsGetChannel((u8*)&tmp);//Normally you don't need to use this.
		printf("udsGetChannel() returned 0x%08x. channel = %u.\n", (unsigned int)ret, (unsigned int)tmp);
		if(R_FAILED(ret))
		{
			return;
		}

		//You can load the appdata with this once connected to the network, if you want.
		memset(out_appdata, 0, sizeof(out_appdata));
		actual_size = 0;
		ret = udsGetApplicationData(out_appdata, sizeof(out_appdata), &actual_size);
		if(R_FAILED(ret) || actual_size!=sizeof(out_appdata))
		{
			printf("udsGetApplicationData() returned 0x%08x. actual_size = 0x%x.\n", (unsigned int)ret, actual_size);
			udsDisconnectNetwork();
			udsUnbind(&bindctx);
			return;
		}

		memset(tmpstr, 0, sizeof(tmpstr));
		if(memcmp(out_appdata, appdata, 4)!=0)
		{
			printf("The first 4-bytes of appdata is invalid.\n");
			udsDisconnectNetwork();
			udsUnbind(&bindctx);
			return;
		}

		strncpy(tmpstr, (char*)&out_appdata[4], sizeof(out_appdata)-5);
		tmpstr[sizeof(out_appdata)-6]='\0';

		printf("String from appdata: %s\n", (char*)&out_appdata[4]);

		con_type = 1;
	}
	else
	{
		udsGenerateDefaultNetworkStruct(&networkstruct, USR_WLANCOMM_ID, 0, UDS_MAXNODES);

		printf("Creating the network...\n");
		ret = udsCreateNetwork(&networkstruct, passphrase, strlen(passphrase)+1, &bindctx, data_channel, recv_buffer_size);
		if(R_FAILED(ret))
		{
			printf("udsCreateNetwork() returned 0x%08x.\n", (unsigned int)ret);
			return;
		}

		ret = udsSetApplicationData(appdata, sizeof(appdata));//If you want to use appdata, you can set the appdata whenever you want after creating the network. If you need more space for appdata, you can set different chunks of appdata over time.
		if(R_FAILED(ret))
		{
			printf("udsSetApplicationData() returned 0x%08x.\n", (unsigned int)ret);
			udsDestroyNetwork();
			udsUnbind(&bindctx);
			return;
		}

		tmp = 0;
		ret = udsGetChannel((u8*)&tmp);//Normally you don't need to use this.
		printf("udsGetChannel() returned 0x%08x. channel = %u.\n", (unsigned int)ret, (unsigned int)tmp);
		if(R_FAILED(ret))
		{
			udsDestroyNetwork();
			udsUnbind(&bindctx);
			return;
		}

		con_type = 0;
	}

	if(udsWaitConnectionStatusEvent(false, false))
	{
		printf("Constatus event signaled.\n");
		NetCORE_Print_ConnStatus();
	}

	printf("Press A to stop data transfer.\n");

	tmpbuf_size = UDS_DATAFRAME_MAXSIZE;
	tmpbuf = malloc(tmpbuf_size);
	if(tmpbuf==NULL)
	{
		printf("Failed to allocate tmpbuf for receiving data.\n");

		if(con_type)
		{
			udsDestroyNetwork();
		}
		else
		{
			udsDisconnectNetwork();
		}
		udsUnbind(&bindctx);

		return;
	}

	while(1)
	{
		gspWaitForVBlank();
		hidScanInput();
		u32 kDown = hidKeysDown();

		if(kDown & KEY_A)break;
		prev_transfer_data = transfer_data;
		transfer_data = hidKeysHeld();

		//When the output from hidKeysHeld() changes, send it over the network.
		if(transfer_data != prev_transfer_data && conntype!=UDSCONTYPE_Spectator)//Spectators aren't allowed to send data.
		{
			ret = udsSendTo(UDS_BROADCAST_NETWORKNODEID, data_channel, UDS_SENDFLAG_Default, &transfer_data, sizeof(transfer_data));
			if(UDS_CHECK_SENDTO_FATALERROR(ret))
			{
				printf("udsSendTo() returned 0x%08x.\n", (unsigned int)ret);
				break;
			}
		}

		//if(udsWaitDataAvailable(&bindctx, false, false))//Check whether data is available via udsPullPacket().
		{
			memset(tmpbuf, 0, tmpbuf_size);
			actual_size = 0;
			src_NetworkNodeID = 0;
			ret = udsPullPacket(&bindctx, tmpbuf, tmpbuf_size, &actual_size, &src_NetworkNodeID);
			if(R_FAILED(ret))
			{
				printf("udsPullPacket() returned 0x%08x.\n", (unsigned int)ret);
				break;
			}

			if(actual_size)//If no data frame is available, udsPullPacket() will return actual_size=0.
			{
				printf("Received 0x%08x size=0x%08x from node 0x%x.\n", (unsigned int)tmpbuf[0], actual_size, (unsigned int)src_NetworkNodeID);
			}
		}

		if(kDown & KEY_Y)
		{
			ret = udsGetNodeInformation(0x2, &tmpnode);//This can be used to get the NodeInfo for a node which just connected, for example.
			if(R_FAILED(ret))
			{
				printf("udsGetNodeInformation() returned 0x%08x.\n", (unsigned int)ret);
			}
			else
			{
				memset(tmpstr, 0, sizeof(tmpstr));

				ret = udsGetNodeInfoUsername(&tmpnode, tmpstr);
				if(R_FAILED(ret))
				{
					printf("udsGetNodeInfoUsername() returned 0x%08x for udsGetNodeInfoUsername.\n", (unsigned int)ret);
				}
				else
				{
					printf("node username: %s\n", tmpstr);
					printf("node unk_x1c=0x%x\n", (unsigned int)tmpnode.unk_x1c);
					printf("node flag=0x%x\n", (unsigned int)tmpnode.flag);
					printf("node pad_x1f=0x%x\n", (unsigned int)tmpnode.pad_x1f);
					printf("node NetworkNodeID=0x%x\n", (unsigned int)tmpnode.NetworkNodeID);
					printf("node word_x24=0x%x\n", (unsigned int)tmpnode.word_x24);
				}
			}
		}

		if(kDown & KEY_X)//Block new regular clients from connecting.
		{
			ret = udsSetNewConnectionsBlocked(true, true, false);
			printf("udsSetNewConnectionsBlocked() for enabling blocking returned 0x%08x.\n", (unsigned int)ret);
		}

		if(kDown & KEY_B)//Unblock new regular clients from connecting.
		{
			ret = udsSetNewConnectionsBlocked(false, true, false);
			printf("udsSetNewConnectionsBlocked() for disabling blocking returned 0x%08x.\n", (unsigned int)ret);
		}

		if(kDown & KEY_R)
		{
			ret = udsEjectSpectator();
			printf("udsEjectSpectator() returned 0x%08x.\n", (unsigned int)ret);
		}

		if(kDown & KEY_L)
		{
			ret = udsAllowSpectators();
			printf("udsAllowSpectators() returned 0x%08x.\n", (unsigned int)ret);
		}

		if(udsWaitConnectionStatusEvent(false, false))
		{
			printf("Constatus event signaled.\n");
			NetCORE_Print_ConnStatus();
		}
	}

	free(tmpbuf);
	tmpbuf = NULL;

	if(con_type)
	{
		udsDestroyNetwork();
	}
	else
	{
		udsDisconnectNetwork();
	}
	udsUnbind(&bindctx);
}


int NetCORE_Example(void)
{
	printf("libctru UDS local-WLAN demo.\n");
	
	NetCORE_UDS_Test();

	return 0;
}