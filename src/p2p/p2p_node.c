#include "p2p/p2p_node.h"

int p2p_handle_request(struct btide_packet *packet_buf, struct package_list
        *package_list, int client_fd) {
    // Retrieve the data in the REQ packet
    uint32_t offset = packet_buf->pl.request.file_offset;
    uint32_t size = packet_buf->pl.request.data_len;
    char hash_buf[SHA256_HEX_STRLEN] = {0};
    strncpy(hash_buf, packet_buf->pl.request.chunk_hash, SHA256_HEX_LEN);
    char ident_buf[MAX_IDENT_SIZE] = {0};
    strncpy(ident_buf, packet_buf->pl.request.ident, IDENT_SIZE);

    // Prepare RES to send back
    packet_buf->msg_code = PKT_MSG_RES;
    int package_index = find_package(package_list, ident_buf,
                                     MIN_IDENT_MATCH);
    // Package is not managed in the application
    if (package_index == -1) {
        send_RES(1, NULL, client_fd);
        return 0;
    }
    // Hash is not in the package
    uint32_t chunk_size = find_hash_in_package
            (package_list->packages[package_index],
                                          hash_buf, offset);
    if (chunk_size == 0) {
        send_RES(1, NULL, client_fd);
        return 0;
    }

    // Keep sending RES until data_len is met
    uint32_t abs_offset = offset;
    uint32_t bytes_sent = 0;
    while (bytes_sent < size) {
        abs_offset += bytes_sent;
        uint32_t remaining = size - bytes_sent;
        union btide_payload res_payload = {0};
        res_payload.response.file_offset = abs_offset;
        strncpy(res_payload.response.chunk_hash, hash_buf, CHUNK_HASH_SIZE);
        strncpy(res_payload.response.ident, ident_buf, IDENT_SIZE);

        // Cannot fit the remaining chunk into the packet
        if (remaining > MAX_DATA_SIZE) {
            get_data(package_list->packages[package_index], MAX_DATA_SIZE,
                     abs_offset, res_payload.response.data);
            res_payload.response.data_len = MAX_DATA_SIZE;

            // Failed to handle REQ
            if (send_RES(0, &res_payload, client_fd) == 0) {
                printf("Client Handler: Failed to send RES\n");
                return 0;
            }
            bytes_sent += MAX_DATA_SIZE;
        } else {
            get_data(package_list->packages[package_index],
                     remaining, abs_offset, res_payload.response
                             .data);
            res_payload.response.data_len = (uint16_t) remaining;

            // Failed to handle REQ
            if (send_RES(0, &res_payload, client_fd) == 0) {
                printf("Client Handler: Failed to send RES\n");
                return 0;
            }
            bytes_sent += remaining;
        }
    }
    return 1;
}

void p2p_handle_response(struct btide_packet *packet_buf, struct package_list
*package_list) {
    // The peer does not have the data requested
    if (packet_buf->error > 0) {
        return;
    }

    // Retrieve the data in the RES packet
    uint32_t offset = packet_buf->pl.response.file_offset;
    uint16_t size = packet_buf->pl.response.data_len;
    char data_buf[MAX_DATA_SIZE] = {0};
    memcpy(data_buf, packet_buf->pl.response.data, size);
    char hash_buf[SHA256_HEX_STRLEN] = {0};
    strncpy(hash_buf, packet_buf->pl.response.chunk_hash, SHA256_HEX_LEN);
    char ident_buf[MAX_IDENT_SIZE] = {0};
    strncpy(ident_buf, packet_buf->pl.response.ident, IDENT_SIZE);

    // Locate the package and file
    int package_index = find_package(package_list, ident_buf,
                                     MIN_IDENT_MATCH);
    // Package is not managed in the application
    if (package_index == -1) {
        return;
    }
    uint32_t chunk_size = find_hash_in_package
            (package_list->packages[package_index],
             hash_buf, offset);
    // Chunk hash is not in the package
    if (chunk_size == 0) {
        return;
    }

    write_data(package_list->packages[package_index], size, offset, data_buf);
}

struct client_handler_args *create_client_handler_args(int peer_fd, char
        *peer_ip, uint16_t peer_port, struct peer_list *peer_list, struct
                package_list *package_list) {
    struct client_handler_args *new_args = calloc(1, sizeof(struct
            client_handler_args));

    struct peer new_peer = {0};
    new_peer.peer_fd = peer_fd;
    strncpy(new_peer.peer_ip, peer_ip, MAX_IP_SIZE);
    new_peer.peer_port = peer_port;

    new_args->new_peer = new_peer;
    new_args->peer_list = peer_list;
    new_args->package_list = package_list;

    return new_args;
}

// Handle connection request from another peer and subsequent packets
void *start_client_handler(void *args) {
    // Retrieve arguments
    struct peer client = ((struct client_handler_args *) args)->new_peer;
    struct peer_list *peer_list = ((struct client_handler_args *) args)
            ->peer_list;
    struct package_list *package_list = ((struct client_handler_args *) args)
            ->package_list;
    free(args);

    // Failed to send ACP or receive ACK
    if (!send_ACP(client.peer_fd)) {
        printf("Failed to send ACP or receive ACK in Client Handler\n");
        pthread_exit((void *)-1);
    }
    add_peer(peer_list, client);

    int client_fd = client.peer_fd;
    struct btide_packet packet_buf = {0};
    // Listen for any packets from the client
    while (1) {
        ssize_t read_result = read(client_fd, &packet_buf, PACKET_SIZE);
        // Client disconnected
        if (read_result <= 0) {
            // remove_peer(peer_list, client.peer_ip, client.peer_port);
            pthread_exit((void *) -1);
        }
        // Try again after reading an invalid packet
        if (read_result < PACKET_SIZE) {
            printf("Invalid packet from Client FD: %d LEN: %lu\n", client_fd,
                   read_result);
            continue;
        }

        // Handle different packet types
        uint16_t msg_code = packet_buf.msg_code;
        if (msg_code == PKT_MSG_REQ) {
            p2p_handle_request(&packet_buf, package_list, client_fd);
            continue;
        } else if (msg_code == PKT_MSG_RES) {
            p2p_handle_response(&packet_buf, package_list);
            continue;
        } else if (msg_code == PKT_MSG_DSN) { // Signal for closing the connection
            remove_peer(peer_list, client.peer_ip, client.peer_port);
            break;
        } else if (msg_code == PKT_MSG_PNG) {
            handle_PNG(client_fd);
            continue;
        } else {
            // Should not receive: ACP, ACK
            // No need to handle: POG
        }
    }

    pthread_exit((void *) 0);
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
        perror("Server: Failed to bind");
        close(server_fd);
        pthread_exit((void *) -1);
    }

    return server_fd;
}

void *start_server(void *args) {
    u_int16_t server_port = ((struct server_args *) args)->port;
    int max_peers = ((struct server_args *) args)->max_peers;
    struct peer_list *peer_list = ((struct server_args *) args)->peer_list;
    struct package_list *package_list = ((struct server_args *) args)
            ->package_list;

    int server_fd = setup_server_socket(server_port);
    // Start listening on the port
    if (listen(server_fd, max_peers) == -1) {
        perror("Server: Failed to listen");
        close(server_fd);
        pthread_exit((void *) -1);
    }

    // Keep handling new connections
    while (1) {
        // Maximum peer capacity reached
        if (peer_list->num_peers >= max_peers) {
            continue;
        }

        int client_fd = 0;
        struct sockaddr_in client_address = {0};
        socklen_t addr_len = sizeof(client_address);
        if ((client_fd = accept(server_fd, (struct sockaddr *)
                &client_address, &addr_len)) == -1) {
            perror("Server: Failed to accept new connection");
            close(server_fd);
            pthread_exit((void *) -1);
        }

        // Create a client handler thread to handle the new peer
        struct client_handler_args *new_args = create_client_handler_args
                (client_fd, inet_ntoa(client_address.sin_addr), ntohs
                        (client_address.sin_port), peer_list, package_list);
        pthread_t handler_thread;
        if (pthread_create(&handler_thread, NULL, start_client_handler,
                           new_args) != 0) {
            free(new_args);
            printf("Failed to create new client handler thread\n");
            continue;
        }
    }

    pthread_exit((void *) 0);
}

struct client_args *create_client_args(char *ip, uint16_t port, struct
        peer_list *peer_list, struct package_list *package_list) {
    struct client_args *new_client_args = calloc(1, sizeof(struct client_args));
    strncpy(new_client_args->ip, ip, MAX_IP_SIZE);
    new_client_args->port = port;
    new_client_args->peer_list = peer_list;
    new_client_args->package_list = package_list;
    return new_client_args;
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
    struct package_list *package_list = ((struct client_args *) args)
            ->package_list;
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
    // Add the peer
    struct peer new_peer = {0};
    new_peer.peer_fd = client_fd;
    inet_ntop(AF_INET, &server_addr.sin_addr, new_peer.peer_ip, MAX_IP_SIZE);
    new_peer.peer_port = ntohs(server_addr.sin_port);
    add_peer(peer_list, new_peer);

    // Listen for any packets from the client
    while (1) {
        ssize_t read_result = read(client_fd, &packet_buf, PACKET_SIZE);
        // Client disconnected
        if (read_result <= 0) {
            // remove_peer(peer_list, new_peer.peer_ip, new_peer.peer_port);
            pthread_exit((void *) -1);
        }
        // Try again after reading an invalid packet
        if (read_result < PACKET_SIZE) {
            printf("Invalid packet from Client FD: %d LEN: %lu\n", client_fd,
                   read_result);
            continue;
        }

        // Handle different packet types
        uint16_t msg_code = packet_buf.msg_code;
        if (msg_code == PKT_MSG_REQ) {
            p2p_handle_request(&packet_buf, package_list, client_fd);
            continue;
        } else if (msg_code == PKT_MSG_RES) {
            p2p_handle_response(&packet_buf, package_list);
            continue;
        } else if (msg_code == PKT_MSG_DSN) { // Signal for closing the connection
            remove_peer(peer_list, new_peer.peer_ip, new_peer.peer_port);
            break;
        } else if (msg_code == PKT_MSG_PNG) {
            handle_PNG(new_peer.peer_fd);
            continue;
        } else {
            // Should not receive: ACP, ACK
            // No need to handle: POG
        }
    }

    pthread_exit((void *) 0);
}
