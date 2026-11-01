#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nccs.h"
#include "nccs_protocol.h"


static NCCS_CentralProfile ctrl_prof;
static NCCS_ClientProfile peer_prof[NCCS_CLIENTS_MAX];


s16 NCCS_Init(size_t sharedmem_size, const char* username)
{
	memset(peer_prof, 0, sizeof(NCCS_ClientProfile)*NCCS_CLIENTS_MAX);
	NCCS_Init_Port(sharedmem_size, username);

	return EXIT_SUCCESS;
}


void NCCS_Exit(void)
{
	NCCS_Exit_Port();
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
	((u8*)segment)[offset]   = (val >> 24) & 0x000000FF;
	((u8*)segment)[offset+1] = (val >> 16) & 0x000000FF;
	((u8*)segment)[offset+2] = (val >> 8)  & 0x000000FF;
	((u8*)segment)[offset+3] = val         & 0x000000FF;
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
	if (seg_size > NCCS_SEGMENT_MAXSIZE) {
		printf("Checksum: segment exceeds MTU (%zu>%zu)\n", seg_size, NCCS_SEGMENT_MAXSIZE);
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