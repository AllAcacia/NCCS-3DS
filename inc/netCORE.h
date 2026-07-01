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


#define NETCORE_CLIENTS_MAX 7      // chosen to reflect a "8-player" session. Note UDS supports 16 total "nodes" in a network.
#define NETCORE_BUF_SEGMENTS_MAX 8 // how many segments can be "buffered" for storage
#define NETCORE_RCVBUF_SIZE (UDS_DATAFRAME_MAXSIZE * NETCORE_BUF_SEGMENTS_MAX)    // in bytes
#define NETCORE_BYTES_PER_WORD 4
#define NETCORE_CMN_HDR_NUM_WORDS 10
#define NETCORE_CMN_HDR_SIZE (NETCORE_BYTES_PER_WORD * NETCORE_CMN_HDR_NUM_WORDS) // in bytes
#define NETCORE_CMN_PYLD_SIZE_MAX (UDS_DATAFRAME_MAXSIZE - NETCORE_CMN_HDR_SIZE)  // in bytes


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
    SEG_INIT=0, // for sender use typically
    SEG_RCVD,
    SEG_RCVD_ACKD,
    SEG_SENT,
    SEG_SENT_ACKD
} NCCS_SegmentState;


typedef struct {
    u16 slot_id;                       // segment slot ID
    u16 nwn_id;                        // related network node ID
    u8 seg_buf[UDS_DATAFRAME_MAXSIZE]; // segment buffer
    u16 seg_size;                      // segment size in bytes
    NCCS_SegmentState seg_state;       // segment "state"
} NCCS_SegmentSlot;


typedef struct {
    bool need_sending;
    NCCS_ConnState conn_state;
    NCCS_SegmentSlot seg_slots[NETCORE_BUF_SEGMENTS_MAX];
    NCCS_SegmentSlot* curr_slot; // to support in-order data scheme, set to NULL if nothing available
    u16 node_id;
    u16 usr_utf16[11]; // UTF-16 encoded username
    u8 usr_utf8[11]; // UTF-8 encoded username
    u64 tsecr; // their clock timestamp (TS echo reply)
    u32 exp_seqno; // expected sequence number
    u32 exp_ackno; // expected acknowledgement number
} NCCS_Client;


int NetCORE_Init(u64 app_id, char* usr_str_utf8);

int NetCORE_Exit(void);

int NetCORE_Execute(void);

int NetCORE_CalculateWlanCommID(void);

u32 NetCORE_GetWlanCommID(void);

int NetCORE_Checksum(const void* array, const size_t size, u16* checksum);

bool NetCORE_GetIsInNetwork(void);

u32 NetCORE_GetConnNetworkID(void);

void NetCORE_Print_ConnStatus(void);

/* ---------------- example from 3ds-examples ---------------- */
void NetCORE_UDS_Test(void);

int NetCORE_Example(void);