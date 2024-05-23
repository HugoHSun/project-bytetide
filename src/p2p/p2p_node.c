#include "p2p/p2p_node.h"

struct client_handler_args *create_chandler_args(int peer_fd, char *peer_ip,
                                                 uint16_t peer_port, struct
                                                 peer_list *peer_list) {
    struct client_handler_args *new_args = calloc(1, sizeof(struct
            client_handler_args));

    struct peer new_peer = {0};
    new_peer.peer_fd = peer_fd;
    strncpy(new_peer.peer_ip, peer_ip, MAX_IP_SIZE);
    new_peer.peer_port = peer_port;

    new_args->new_peer = new_peer;
    new_args->peer_list = peer_list;

    return new_args;
}

struct client_args *create_client_args(char *ip, uint16_t port, struct peer_list *peer_list) {
    struct client_args *new_client_args = calloc(1, sizeof(struct client_args));
    strncpy(new_client_args->ip, ip, MAX_IP_SIZE);
    new_client_args->port = port;
    new_client_args->peer_list = peer_list;
    return new_client_args;
}

int setup_server_socket(u_int16_t port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Server: Failed to create socket");
        pthread_exit((void *) -1);
    }

    struct sockaddr_in socket_addr;
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(port);
    socket_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_fd, (struct sockaddr *) &socket_addr, sizeof
            (struct sockaddr_in)) == -1) {
        printf("Server: Failed to bind\n");
        close(server_fd);
        pthread_exit((void *) -1);
    }

    return server_fd;
}

// Handle connect request from other peers
void *client_handler(void *args) {
    struct peer client = ((struct client_handler_args *) args)->new_peer;
    struct peer_list *peer_list = ((struct client_handler_args *) args)
            ->peer_list;
    free(args);

    // Failed to send ACP or receive ACK
    if (!send_ACP(client)) {
        printf("Failed to send ACP or receive ACK in Client Handler\n");
        pthread_exit((void *)-1);
    }
    add_peer(peer_list, client);

    // todo
    int client_fd = client.peer_fd;
    struct btide_packet packet_buf = {0};
    while (1) {
        ssize_t read_result = read(client_fd, &packet_buf, PAYLOAD_MAX);
        // Client disconnected
        if (read_result <= 0) {
            remove_peer(peer_list, client.peer_ip, client.peer_port);
            pthread_exit((void *) 0);
        }
        if (read_result < PAYLOAD_MAX) {
            printf("Invalid packet at Client FD: %d\n", client_fd);
            continue;
        }

        packet_handler(&packet_buf, client);
    }
}

void *start_server(void *args) {
    u_int16_t server_port = ((struct server_args *) args)->port;
    int max_peers = ((struct server_args *) args)->max_peers;
    struct peer_list *peer_list = ((struct server_args *) args)->peer_list;

    int server_fd = setup_server_socket(server_port);
    // Start listening on the port
    if (listen(server_fd, max_peers) == -1) {
        printf("Server: Failed to listen\n");
        close(server_fd);
        pthread_exit((void *) -1);
    }

    while (1) {
        if (peer_list->num_peers >= max_peers) {
            continue;
        }

        int client_fd = 0;
        struct sockaddr_in client_address = {0};
        socklen_t addr_len = sizeof(client_address);
        if ((client_fd = accept(server_fd, (struct sockaddr *)
                &client_address, &addr_len)) == -1) {
            printf("Server: Failed to accept new connection\n");
            close(server_fd);
            pthread_exit((void *) -1);
        }

        // Create a client handler thread to handle every new connection
        struct client_handler_args *new_args = create_chandler_args
                (client_fd, inet_ntoa(client_address.sin_addr), ntohs
                (client_address.sin_port), peer_list);
        pthread_t handler_thread;
        if (pthread_create(&handler_thread, NULL, client_handler,
                           new_args) != 0) {
            free(new_args);
            printf("Failed to create new client handler thread\n");
            continue;
        }
    }

    pthread_exit((void *) 0);
}

// Client Thread
void *start_client(void *args) {
    // Retrieve arguments
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(((struct client_args *) args)->port);
    if (inet_pton(AF_INET, ((struct client_args *) args)->ip, &server_addr.sin_addr) <= 0) {
        printf("Unable to connect to request peer\n");
        free(args);
        pthread_exit((void *) -1);
    }
    struct peer_list *peer_list = ((struct client_args *) args)->peer_list;
    free(args);

    // Set up client socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Client: Failed to create socket");
        pthread_exit((void *) -1);
    }

    // Connect to the server
    if (connect(client_fd, (struct sockaddr *) &server_addr, sizeof
            (server_addr)) == -1) {
        perror("Client: Failed to connect to server");
        close(client_fd);
        pthread_exit((void *) -1);
    }

    struct btide_packet packet_buf = {0};
    // Wait for ACP packet
    if (!get_packet_tm(&packet_buf, client_fd)) {
        printf("Unable to connect to request peer\n");
        close(client_fd);
        pthread_exit((void *) -1);
    }
    // Send ACK packet
    if (!handle_ACP(client_fd)) {
        printf("Unable to connect to request peer\n");
        close(client_fd);
        pthread_exit((void *) -1);
    }

    printf("Connection established with peer\n");
    struct peer new_peer = {0};
    new_peer.peer_fd = client_fd;
    inet_ntop(AF_INET, &server_addr.sin_addr, new_peer.peer_ip, MAX_IP_SIZE);
    new_peer.peer_port = ntohs(server_addr.sin_port);
    add_peer(peer_list, new_peer);

    // todo

    pthread_exit((void *) 0);
}
