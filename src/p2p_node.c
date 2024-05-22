#include "p2p/p2p_node.h"

int setup_server_socket(u_int16_t port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        printf("Server: Failed to create socket\n");
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

void *start_server(void *args) {
    int server_fd = setup_server_socket(((struct server_args *) args)->port);
    // Start listening on the port
    if (listen(server_fd, ((struct server_args *) args)->max_peers) == -1) {
        printf("Server: Failed to listen\n");
        close(server_fd);
        pthread_exit((void *) -1);
    }
    printf("Server Started on port: %d\n", ((struct
            server_args *) args)->port);

    while (1) {
        int client_fd = 0;
        struct sockaddr_in client_address = {0};
        socklen_t addr_len = sizeof(client_address);
        if ((client_fd = accept(server_fd, (struct sockaddr *)
                &client_address, &addr_len)) == -1) {
            printf("Server: Failed to accept new connection\n");
            close(server_fd);
            pthread_exit((void *) -1);
        }

        // Add the new peer to peer list
        struct peer new_peer = {0};
        new_peer.peer_fd = client_fd;
        strncpy(new_peer.peer_ip, inet_ntoa(client_address.sin_addr),
                MAX_IP_SIZE);
        new_peer.peer_port = ntohs(client_address.sin_port);
        add_peer(((struct server_args *) args)->peer_list, new_peer);

        printf("Connected to Client: socket fd: %d, IP: %s, port: %d\n",
               client_fd, inet_ntoa(client_address.sin_addr),
               ntohs(client_address.sin_port));
    }

    pthread_exit((void *) 0);
}

void *start_client(void *args) {
    struct sockaddr_in server_addr = ((struct client_args *) args)->server_addr;

    // Set up client socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        printf("Client: Failed to create socket\n");
        pthread_exit((void *) -1);
    }

    // Connect to the server
    if (connect(client_fd, (struct sockaddr *) &server_addr, sizeof
            (server_addr)) == -1) {
        printf("Client: Failed to connect to server\n");
        close(client_fd);
        pthread_exit((void *) -1);
    }

    printf("Connected to Server: IP: %s, port: %d\n", inet_ntoa
            (server_addr.sin_addr), ntohs(server_addr.sin_port));

    pthread_exit((void *) 0);
}
