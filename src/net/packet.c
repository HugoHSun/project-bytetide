#include "net/packet.h"

/**
 * Send a packet to peer
 * @param msg_code
 * @param payload
 * @param peer_fd
 * @return 1 if success, 0 for error
 */
int send_packet(uint16_t msg_code, union btide_payload *payload, int peer_fd) {
    struct btide_packet packet_buf = {0};
    packet_buf.msg_code = msg_code;

    if (payload != NULL && msg_code == PKT_MSG_REQ) {
        packet_buf.pl.request = payload->request;
    } else if (payload != NULL && msg_code == PKT_MSG_RES) {
        packet_buf.pl.response = payload->response;
    }

    size_t send_result = send(peer_fd, &packet_buf, PACKET_SIZE, 0);

    // The socket connection is disconnected
    if (send_result == -1) {
        close(peer_fd);
        return 0;
    }

    if (send_result < PACKET_SIZE) {
        printf("Failed to send all contents in %hu Packet to client: %d\n",
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
    if (read_result == -1) {
        close(peer_fd);
        return 0;
    }

    // Change the socket back to waiting indefinitely
    timeout.tv_sec = 0;
    setsockopt(peer_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout,
               sizeof(timeout));

    if (read_result < PACKET_SIZE) {
        printf("Invalid Packet read from client: %d\n", peer_fd);
        return 0;
    }
    return 1;
}

int send_ACP(struct peer peer) {
    int peer_fd = peer.peer_fd;

    // Send the ACP packet
    if (!send_packet(PKT_MSG_ACP, NULL, peer_fd)) {
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
    if (!send_packet(PKT_MSG_ACK, NULL, peer_fd)) {
        printf("Failed to send ACK Packet to client: %d\n", peer_fd);
        return 0;
    }
    return 1;
}

int send_DSN(int peer_fd) {
    // Send the DSN packet
    if (!send_packet(PKT_MSG_DSN, NULL, peer_fd)) {
        return 0;
    }

    close(peer_fd);
    return 1;
}

void handle_DSN(int peer_fd) {
    close(peer_fd);
}

int send_REQ(union btide_payload *req, int peer_fd) {
    if (!send_packet(PKT_MSG_REQ, req, peer_fd)) {
        return 0;
    }

    struct btide_packet response_packet = {0};
    get_packet_tm(&response_packet, peer_fd);
    return 0;
}

void handle_REQ() {

}

int send_PNG(int peer_fd) {
    if (!send_packet(PKT_MSG_PNG, NULL, peer_fd)) {
        return 0;
    }

    struct btide_packet packet_buf = {0};
    if (!get_packet_tm(&packet_buf, peer_fd)) {
        return 0;
    }

    return 1;
}

void handle_PNG(int peer_fd) {
    if (!send_packet(PKT_MSG_POG, NULL, peer_fd)) {
        printf("Failed to send POG Packet to client FD: %d\n", peer_fd);
    }
}

void packet_handler(struct btide_packet *packet_buf, struct peer peer) {
    uint16_t msg_code = packet_buf->msg_code;

    if (msg_code == PKT_MSG_ACP) {
        handle_ACP(peer.peer_fd);
        return;
    }

    if (msg_code == PKT_MSG_ACK) {
        printf("RECEIVED ACK PACKET");
        return;
    }

    if (msg_code == PKT_MSG_DSN) {
        handle_DSN(peer.peer_fd);
        return;
    }

    if (msg_code == PKT_MSG_REQ) {
        printf("RECEIVED REQ from Client fd: %d\n", peer.peer_fd);
        return;
    }

    if (msg_code == PKT_MSG_RES) {
        return;
    }

    if (msg_code == PKT_MSG_PNG) {
        send_PNG(peer.peer_fd);
        return;
    }

    if (msg_code == PKT_MSG_POG) {
        return;
    }
}
