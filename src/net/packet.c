#include "net/packet.h"

/**
 *  * Send a packet to a peer
 * @param msg_code
 * @param err
 * @param payload
 * @param peer_fd
 * @return 1 if success, 0 for error
 */
int send_packet(uint16_t msg_code, uint16_t err, union btide_payload *payload,
                int peer_fd) {
    struct btide_packet packet_buf = {0};
    packet_buf.msg_code = msg_code;
    packet_buf.error = err;

    if (payload != NULL && msg_code == PKT_MSG_REQ) {
        packet_buf.pl.request = payload->request;
    } else if (payload != NULL && msg_code == PKT_MSG_RES) {
        packet_buf.pl.response = payload->response;
    }

    size_t send_result = send(peer_fd, &packet_buf, PACKET_SIZE, 0);
    // The socket is disconnected
    if (send_result <= 0) {
        return 0;
    }

    if (send_result < PACKET_SIZE) {
        printf("Failed to send all contents in %hu Packet to Client FD: %d\n",
               msg_code, peer_fd);
        return 0;
    }

    return 1;
}

/**
 * Wait for a packet with a timeout of 3 seconds
 * @param packet_buf
 * @param peer_fd
 * @return
 */
int get_packet_tm(struct btide_packet *packet_buf, int peer_fd) {
    // 3 seconds timeout for receiving a response
    struct timeval timeout = {3, 0};
    setsockopt(peer_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout,
               sizeof(timeout));

    ssize_t read_result = read(peer_fd, packet_buf, PACKET_SIZE);
    // Peer disconnected or error occurred
    if (read_result <= 0) {
        return 0;
    }
    // Invalid packet read
    if (read_result < PACKET_SIZE) {
        printf("Invalid Packet read from Client FD: %d LEN: %lu\n", peer_fd,
               read_result);
        return 0;
    }

    // Change the socket back to waiting indefinitely
    timeout.tv_sec = 0;
    setsockopt(peer_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout,
               sizeof(timeout));
    return 1;
}

int send_ACP(int peer_fd) {
    // Send the ACP packet
    if (!send_packet(PKT_MSG_ACP, 0, NULL, peer_fd)) {
        return 0;
    }

    // 3 seconds timeout for receiving a response
    struct btide_packet packet_buf = {0};
    if (!get_packet_tm(&packet_buf, peer_fd)) {
        return 0;
    }

    return 1;
}

int handle_ACP(int peer_fd) {
    if (!send_packet(PKT_MSG_ACK, 0, NULL, peer_fd)) {
        printf("Failed to send ACK Packet to Peer FD: %d\n", peer_fd);
        return 0;
    }
    return 1;
}

int send_DSN(int peer_fd) {
    // Send the DSN packet
    if (!send_packet(PKT_MSG_DSN, 0, NULL, peer_fd)) {
        printf("Failed to send DSN Packet to Peer FD: %d\n", peer_fd);
        return 0;
    }

    return 1;
}

int send_REQ(union btide_payload *req, int peer_fd) {
    if (!send_packet(PKT_MSG_REQ, 0, req, peer_fd)) {
        printf("Failed to send REQ Packet to Peer FD: %d\n", peer_fd);
        free(req);
        return 0;
    }

    return 1;
}

int send_RES(uint16_t err, union btide_payload *res, int peer_fd) {
    if (!send_packet(PKT_MSG_RES, err, res, peer_fd)) {
        printf("Failed to send RES packet to Peer FD: %d\n", peer_fd);
        free(res);
        return 0;
    }

    return 1;
}

int send_PNG(int peer_fd) {
    if (!send_packet(PKT_MSG_PNG, 0, NULL, peer_fd)) {
        printf("Failed to send PNG packet to Peer FD: %d\n", peer_fd);
        return 0;
    }

    // Wait for POG with 3 seconds timeout
    struct btide_packet packet_buf = {0};
    if (!get_packet_tm(&packet_buf, peer_fd)) {
        return 0;
    }
    return 1;
}

int handle_PNG(int peer_fd) {
    if (!send_packet(PKT_MSG_POG, 0, NULL, peer_fd)) {
        printf("Failed to send POG Packet to Peer FD: %d\n", peer_fd);
        return 0;
    }
    return 1;
}

/**
 * Handle received packets with no payload
 * @param packet_buf
 * @param peer
 * @return 1 if successfully handled, 0 when handling failed, -1 to signal
 * remove peer
 */
int packet_handler_non_payload(uint16_t msg_code, int peer_fd) {
    if (msg_code == PKT_MSG_ACP) {
        return handle_ACP(peer_fd);
    }

    if (msg_code == PKT_MSG_ACK) {
        return 1;
    }

    if (msg_code == PKT_MSG_DSN) {
        return -1;
    }

    if (msg_code == PKT_MSG_PNG) {
        return handle_PNG(peer_fd);
    }

    if (msg_code == PKT_MSG_POG) {
        return 1;
    }

    printf("Packet not recognised\n");
    return 0;
}
