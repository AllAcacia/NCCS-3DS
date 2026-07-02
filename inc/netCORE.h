/*
 * Filename: netCORE.h
 * Author:   AllAcacia
 * 
 * MODULE INFO:
 * Handles inter-console
 * connectivity.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <3ds.h>

// Constant NCCS Macros
#define NCCS_CLIENTS_MAX 15     // UDS supports 16 total "nodes" in a network.
#define NCCS_MAIN_RCVBUF_BUFSIZE_MAX UDS_DATAFRAME_MAXSIZE
#define NCCS_SNDBUF_SEG_MAX 16
#define NCCS_RCVBUF_SEG_MAX 16

/*
                    NCCS Common Network Header

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         Application ID                        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|      Srce Network Node ID     |      Dest Network Node ID     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   Srce WLAN Communication ID                  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   Dest WLAN Communication ID                  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                             Seqno.                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                             Ackno.                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Application Msg Type      |S|A|N|F|       Reserved        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Payload Length         |           Checksum            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               :
:                             Data                              :
:                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
// NCCS Header Macros
#define NCCS_BYTES_PER_WORD 4
#define NCCS_HDR_NUM_WORDS 8
#define NCCS_HDR_LEN (NCCS_BYTES_PER_WORD * NCCS_HDR_NUM_WORDS)  // in bytes
#define NCCS_PYLD_LEN_MAX (UDS_DATAFRAME_MAXSIZE - NCCS_HDR_LEN) // in bytes

#define NCCS_APPID_BYTE_OFFSET 0
#define NCCS_SRCENWNID_BYTE_OFFSET 4
#define NCCS_DESTNWNID_BYTE_OFFSET 6
#define NCCS_SRCEWLANID_BYTE_OFFSET 8
#define NCCS_DESTWLANID_BYTE_OFFSET 12
#define NCCS_SEQNO_BYTE_OFFSET 16
#define NCCS_ACKNO_BYTE_OFFSET 20
#define NCCS_APPMSGTYPE_BYTE_OFFSET 24
#define NCCS_SYNFLAG_BYTE_OFFSET 26
#define NCCS_SYNFLAG_BIT_N 7
#define NCCS_ACKFLAG_BYTE_OFFSET 26
#define NCCS_ACKFLAG_BIT_N 6
#define NCCS_NAKFLAG_BYTE_OFFSET 26
#define NCCS_NAKFLAG_BIT_N 5
#define NCCS_FINFLAG_BYTE_OFFSET 26
#define NCCS_FINFLAG_BIT_N 4
#define NCCS_PAYLOADLEN_BYTE_OFFSET 28
#define NCCS_CHECKSUM_BYTE_OFFSET 30


typedef enum {
    CONN_CLOSED=0,    // initial
    CONN_LISTEN,      // passive open state
    CONN_SYN_RCVD,    // received a SYN, respond with SYN+ACK and wait for ACK
    CONN_SYN_SENT,    // for SYN sender, waiting for SYN+ACK after sending SYN
    CONN_ESTABLISHED, // after both peers of a connection receive last setup segment
    CONN_CLOSE_WAIT,  // after receiving a close initiation with FIN
    CONN_LAST_ACK,    // entered from close-wait, after sending previous FIN we wait for an ACK
    CONN_FIN_WAIT_1,  // for the closing initiator from established state, after sending previous FIN we wait for an ACK
    CONN_FIN_WAIT_2,  // for the closing initiator, when we receive ACK from our previous FIN
    CONN_CLOSING,     // for simultaneous close?
    CONN_TIME_WAIT    // temporary state for when you want to make sure both sides of conn have no objections before closing
} NCCS_ConnState; // Taken from TCP state machine


typedef enum {
    SLOT_NOTUSED=0,   // free to use
    SLOT_UNSENT,      // 
    SLOT_SENT_UNACKD,
    SLOT_RCVD_UNREAD,
} NCCS_SlotState;


typedef struct {
    u16 slot_id;                       // segment slot ID
    u16 seg_size;                      // segment size in bytes
    u8 seg_buf[UDS_DATAFRAME_MAXSIZE]; // segment buffer
    NCCS_SlotState slot_state;         // segment "state"
} NCCS_SegmentSlot;


typedef struct {
    bool ready_to_send; // flag to indicate data needs sending
    NCCS_ConnState conn_state;

    u32 comm_id;       // WLAN CommID is determined on a per-console basis to be unique
    u16 node_id;       // NodeID is determined through the network
    
    u16 usr_utf16[11]; // UTF-16 encoded username
    u8 usr_utf8[11]; // UTF-8 encoded username

    NCCS_SegmentSlot snd_slots[NCCS_SNDBUF_SEG_MAX];
    NCCS_SegmentSlot rcv_slots[NCCS_RCVBUF_SEG_MAX];
    NCCS_SegmentSlot* curr_rcv_slot; // to support in-order data scheme, set to NULL if nothing available
    
    u32 snd_una;      // number of next seqno that is sent but not ack'd
    u32 snd_nxt;      // number of next seqno to send
    u32 rcv_nxt;      // number of next seqno to receive
    u32 rcv_read_nxt; // number of next seqno to read from app layer, assumes they are read quicker than received

    u16 snd_slot_nxt; // next available sending slot index
    u16 rcv_slot_nxt; // next available receiving slot index

    u16 snd_wnd; // sending window size
    u16 rcv_wnd; // sending window size
} NCCS_NetworkPeer;


int NetCORE_Init(u64 app_id, char* usr_str_utf8, u8 max_clients);

int NetCORE_Exit(void);

int NetCORE_Execute(void);

int NetCORE_CalculateWlanCommID(void);

u32 NetCORE_GetWlanCommID(void);

int NCCS_CalcChecksum(const void* segment, size_t seg_size, u16* checksum);

u8 NCCS_GetBE1b(const void* segment, size_t byte_offset, u8 bit_n);

int NCCS_SetBE1b(void* segment, size_t byte_offset, u8 bit_n, u8 val);

u16 NCCS_GetBE16b(const void* segment, size_t offset);

void NCCS_SetBE16b(void* segment, size_t offset, u16 val);

u32 NCCS_GetBE32b(const void* segment, size_t offset);

void NCCS_SetBE32b(void* segment, size_t offset, u32 val);

u32 NCCS_GetAppID(const void* segment);

void NCCS_SetAppID(void* segment, u32 app_id);

u16 NCCS_GetSrceNWNID(const void* segment);

void NCCS_SetSrceNWNID(void* segment, u16 srce_nwnid);

u16 NCCS_GetDestNWNID(const void* segment);

void NCCS_SetDestNWNID(void* segment, u16 dest_nwnid);

u32 NCCS_GetSrceWLANID(const void* segment);

void NCCS_SetSrceWLANID(void* segment, u32 srce_wlanid);

u32 NCCS_GetDestWLANID(const void* segment);

void NCCS_SetDestWLANID(void* segment, u32 dest_wlanid);

u32 NCCS_GetSeqno(const void* segment);

void NCCS_SetSeqno(void* segment, u32 seqno);

u32 NCCS_GetAckno(const void* segment);

void NCCS_SetAckno(void* segment, u32 ackno);

u16 NCCS_GetAppMsgType(const void* segment);

void NCCS_SetAppMsgType(void* segment, u16 appmsgtype);

u8 NCCS_GetSynFlag(const void* segment);

void NCCS_SetSynFlag(void* segment, u8 syn);

u8 NCCS_GetAckFlag(const void* segment);

void NCCS_SetAckFlag(void* segment, u8 ack);

u8 NCCS_GetNakFlag(const void* segment);

void NCCS_SetNakFlag(void* segment, u8 nak);

u8 NCCS_GetFinFlag(const void* segment);

void NCCS_SetFinFlag(void* segment, u8 fin);

u16 NCCS_GetPayloadLen(const void* segment);

void NCCS_SetPayloadLen(void* segment, u16 payload_len);

u16 NCCS_GetChecksum(const void* segment);

void NCCS_SetChecksum(void* segment, u16 checksum);

bool NetCORE_GetIsInNetwork(void);

u32 NetCORE_GetConnNetworkID(void);

u16 NetCORE_GetConnNodeID(void);

void NetCORE_Print_ConnStatus(void);

/* ---------------- example from 3ds-examples ---------------- */
void NetCORE_UDS_Test(void);

int NetCORE_Example(void);