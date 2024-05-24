#ifndef NETPKT_H
#define NETPKT_H

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <bits/types/struct_timeval.h>

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
    struct request_payload request;
    struct response_payload response;
};

struct btide_packet {
    uint16_t msg_code;
    uint16_t error;
    union btide_payload pl;
};

int get_packet_tm(struct btide_packet *packet_buf, int peer_fd);

/**
 * Send ACP to peer and wait for ACK with 3 seconds timeout
 * @param peer_fd
 * @return 1 if success, 0 otherwise
 */
int send_ACP(int peer_fd);

/**
 * Handle ACP by sending back ACK
 * @param peer_fd
 * @return 1 if success, 0 otherwise
 */
int handle_ACP(int peer_fd);

/**
 * Send DSN to peer
 * @param peer_fd
 * @return 1 if success, 0 otherwise
 */
int send_DSN(int peer_fd);

/**
 * Send REQ to peer
 * @param req REQ payload
 * @param peer_fd
 * @return 1 if success, 0 otherwise
 */
int send_REQ(union btide_payload *req, int peer_fd);

/**
 * Send RES to peer
 * @param res the RES payload to be send back
 * @param peer_fd
 * @return 1 if success, 0 otherwise
 */
int send_RES(uint16_t err, union btide_payload *res, int peer_fd);

/**
 * Send PNG
 * @param peer_fd
 * @return 1 if success, 0 otherwise
 */
int send_PNG(int peer_fd);

/**
 * Send POG
 * @param peer_fd
 * @return
 */
int handle_PNG(int peer_fd);

#endif
