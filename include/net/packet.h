#ifndef NETPKT_H
#define NETPKT_H

#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bits/types/struct_timeval.h>

#include "p2p/peer.h"

#define PAYLOAD_MAX (4092)
#define PACKET_SIZE 4096
#define IDENT_SIZE 1024
#define CHUNK_HASH_SIZE 64
#define MAX_DATA_SIZE 2998

#define PKT_MSG_ACK 0x0c
#define PKT_MSG_ACP 0x02
#define PKT_MSG_DSN 0x03
#define PKT_MSG_REQ 0x06
#define PKT_MSG_RES 0x07
#define PKT_MSG_PNG 0xFF
#define PKT_MSG_POG 0x00

struct request_payload {
    uint32_t file_offset;
    uint16_t data_len;
    char chunk_hash[CHUNK_HASH_SIZE];
    char ident[IDENT_SIZE];
};

struct response_payload {
    uint32_t file_offset;
    char data[MAX_DATA_SIZE];
    uint16_t data_len;
    char chunk_hash[CHUNK_HASH_SIZE];
    char ident[IDENT_SIZE];
};

union btide_payload {
    uint8_t data[PAYLOAD_MAX];
    struct request_payload request;
    struct response_payload response;
};

struct btide_packet {
    uint16_t msg_code;
    uint16_t error;
    union btide_payload pl;
};

/**
 * Send ACP packet to peer
 * @param peer
 * @return 1 if successful sent ACP and received ACK in 3 seconds, 0 otherwise
 */
int send_ACP(struct peer peer);

int get_packet_tm(struct btide_packet *packet_buf, int peer_fd);

void packet_handler(struct btide_packet *packet_buf, struct peer peer);

#endif
