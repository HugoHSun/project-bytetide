#include "p2p/peer.h"

struct peer_list *create_peer_list() {
    int init_size = PEERS_INIT_SIZE;
    struct peer_list *new_list = calloc(1, sizeof(struct peer_list));
    new_list->max_size = init_size;
    new_list->num_peers = 0;
    new_list->peers = calloc(init_size, sizeof(struct peer));

    struct peer init_peer = {0};
    init_peer.peer_fd = -1;
    for (int i = 0; i < init_size; ++i) {
        new_list->peers[i] = init_peer;
    }

    return new_list;
}

void add_peer(struct peer_list *list, struct peer new_peer) {
    // Double the max size when capacity is almost reached
    if ((list->max_size - 1) == list->num_peers) {
        int old_size = list->max_size;
        list->max_size *= 2;
        list->peers = realloc(list->peers, list->max_size * sizeof(struct
        peer));

        struct peer init_peer = {0};
        init_peer.peer_fd = -1;
        for (int i = old_size; i < list->max_size; ++i) {
            list->peers[i] = init_peer;
        }
    }

    for (int i = 0; i < list->max_size; ++i) {
        if (list->peers[i].peer_fd == -1) {
            list->peers[i] = new_peer;
            list->num_peers++;
            return;
        }
    }

    printf("peer.c: add_peer: ERROR\n");
}

/**
 * Find the index of the peer in the list
 * @param list
 * @param peer
 * @return index of the peer, -1 otherwise
 */
int find_peer(struct peer_list *list, char *ip, u_int16_t port) {
    for (int i = 0; i < list->max_size; ++i) {
        struct peer current_peer = list->peers[i];
        if (current_peer.peer_fd != -1 && (strncmp(current_peer.peer_ip, ip,
            MAX_IP_SIZE) == 0) && current_peer.peer_port == port) {
            return i;
        }
    }
    return -1;
}

void remove_peer(struct peer_list *list, char *ip, u_int16_t port) {
    int index;
    if ((index = find_peer(list, ip, port)) == -1) {
        return;
    }

    // Close the peer socket
    close(list->peers[index].peer_fd);
    list->peers[index].peer_fd = -1;
    list->num_peers--;
}

void print_peer_list(struct peer_list *list) {
    int print_count = 0;
    for (int i = 0; i < list->max_size; ++i) {
        struct peer current_peer = list->peers[i];
        if (current_peer.peer_fd != -1) {
            print_count++;
            if (print_count == 1) {
                printf("Connected to:\n\n");
            }
            printf("%d. %s:%hu\n", print_count, current_peer.peer_ip,
                   current_peer.peer_port);
        }
    }

    if (print_count == 0) {
        printf("Not connected to any peers\n");
    }

    // Debug check
    if (print_count != list->num_peers) {
        printf("peer.c: Did not print all peers\n");
    }
}

void free_peer_list(struct peer_list *list) {
    if (list == NULL) {
        return;
    }

    free(list->peers);
    free(list);
}
