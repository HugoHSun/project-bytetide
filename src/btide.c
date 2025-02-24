#include "config/config.h"
#include "p2p/p2p_node.h"

#define MAX_BTIDE_LINE_SIZE 5521
#define MAX_COMMAND_SIZE 16
#define IP_BUFFER_SIZE 16
#define MIN_IDENT_SIZE 20

int main(int argc, char **argv) {
    // Load the configuration file
    struct config config = {0};
    int result = parse_config(argv[1], &config);
    if (result != 0) {
        printf("btide: Failed to read configuration\n");
        return result;
    }

    // Peer and package management structure
    struct peer_list *peer_list = create_peer_list();
    struct package_list *package_list = create_package_list();

    // Start the server in a new thread
    struct server_args args = {config.max_peers, config.port, peer_list, package_list};
    pthread_t server_thread;
    if (pthread_create(&server_thread, NULL, start_server, &args) != 0) {
        printf("btide: Failed to start server\n");
        free_peer_list(peer_list);
        free_package_list(package_list);
        return -1;
    }

    // Command line interface
    char current_line[MAX_BTIDE_LINE_SIZE] = {0};
    while (fgets(current_line, MAX_BTIDE_LINE_SIZE, stdin) != NULL) {
        char command_buf[MAX_COMMAND_SIZE] = {0};
        sscanf(current_line, "%15s", command_buf);

        // Can have '\n' at the end
        if (strncmp(command_buf, "QUIT", MAX_COMMAND_SIZE) == 0 && (strlen
        (current_line) == 4 || strlen(current_line) == 5)) {
            break;
        }

        if (strncmp(command_buf, "PACKAGES", MAX_COMMAND_SIZE) == 0 && (strlen
        (current_line) == 8 || strlen(current_line) == 9)) {
            print_package_list(package_list);
            continue;
        }

        if (strncmp(command_buf, "PEERS", MAX_COMMAND_SIZE) == 0 && (strlen
        (current_line) == 5 || strlen(current_line) == 6)) {
            // Send PNG to all peers
            for (int i = 0; i < peer_list->max_size; ++i) {
                if (peer_list->peers[i].peer_fd > 0) {
                    send_PNG(peer_list->peers[i].peer_fd);
                }
            }
            print_peer_list(peer_list);
            continue;
        }

        // Append a space at the end of the command string for strcmp
        for (int i = 0; i < (MAX_COMMAND_SIZE - 1); ++i) {
            if (command_buf[i] == '\0') {
                command_buf[i] = ' ';
                break;
            }
        }

        if (strncmp(command_buf, "CONNECT ", MAX_COMMAND_SIZE) == 0) {
            char ip_buf[IP_BUFFER_SIZE] = {0};
            int port_buf = 0;
            if (sscanf(current_line, "%15[^0-9]%15[^:]:%d", command_buf,
                       ip_buf, &port_buf) != 3 || strncmp(command_buf,
                       "CONNECT ", MAX_COMMAND_SIZE) != 0) {
                printf("Missing address and port argument\n");
                continue;
            }

            // Invalid port number
            if (port_buf < MIN_PORT_NUM || port_buf > MAX_PORT_NUM) {
                printf("Invalid Input\n");
                continue;
            }

            if (find_peer(peer_list, ip_buf, port_buf) != -1) {
                printf("Already connected to peer\n");
                continue;
            }

            // Maximum number of peers reached
            if (peer_list->num_peers >= config.max_peers) {
                printf("Unable to connect to request peer\n");
                continue;
            }

            // Make a new client thread to connect the new peer
            struct client_args *new_args = create_client_args(ip_buf,
                    port_buf, peer_list, package_list);
            pthread_t client_thread;
            if (pthread_create(&client_thread, NULL, start_client, new_args) != 0) {
                printf("btide: Failed to start client\n");
            }
            continue;
        }

        if (strncmp(command_buf, "DISCONNECT ", MAX_COMMAND_SIZE) == 0) {
            char ip_buf[IP_BUFFER_SIZE] = {0};
            int port_buf = 0;
            if (sscanf(current_line, "%15[^0-9]%15[^:]:%d", command_buf,
                       ip_buf, &port_buf) != 3 || strncmp(command_buf,
                       "DISCONNECT ", MAX_COMMAND_SIZE) != 0) {
                printf("Missing address and port argument\n");
                continue;
            }

            // Invalid port number
            if (port_buf < MIN_PORT_NUM || port_buf > MAX_PORT_NUM) {
                printf("Invalid Input\n");
                continue;
            }

            int index;
            if ((index = find_peer(peer_list, ip_buf, port_buf)) == -1) {
                printf("Unknown peer, not connected\n");
                continue;
            }

            send_DSN(peer_list->peers[index].peer_fd);
            remove_peer(peer_list, ip_buf, port_buf);
            printf("Disconnected from peer\n");
            continue;
        }

        if (strncmp(command_buf, "ADDPACKAGE ", MAX_COMMAND_SIZE) == 0) {
            char space_buf = 0;
            char filename_buf[MAX_FILENAME_SIZE] = {0};
            if (sscanf(current_line, "%15s%c%256[^\n]", command_buf,
                       &space_buf, filename_buf) != 3 || space_buf != ' ') {
                printf("Missing file argument\n");
                continue;
            }

            if (!check_file_existence(filename_buf)) {
                printf("Cannot open file\n");
                continue;
            }

            struct bpkg_obj *package = bpkg_load_no_message(filename_buf,
                    config.directory);
            if (package == NULL) {
                printf("Unable to parse bpkg file\n");
                continue;
            }

            // Create the data file if it doesn't exist
            char data_full_path[MAX_DIRECTORY_SIZE + MAX_FILENAME_SIZE];
            get_file_full_path(data_full_path, package);
            if (!check_file_existence(data_full_path)) {
                FILE *fp = NULL;
                if ((fp = fopen(data_full_path, "wb")) == NULL) {
                    perror("fopen - wb:");
                }
                // Create file with specified size
                char null_byte = 0;
                for (size_t i = 0; i < package->size; ++i) {
                    fwrite(&null_byte, sizeof(char), 1, fp);
                }
                fclose(fp);
            }

            add_package(package_list, package);
            continue;
        }

        if (strncmp(command_buf, "REMPACKAGE ", MAX_COMMAND_SIZE) == 0) {
            char space_buf = 0;
            char ident_buf[MAX_IDENT_SIZE] = {0};
            if (sscanf(current_line, "%15s%c%1024s", command_buf, &space_buf,
                       ident_buf) != 3 || space_buf != ' ' || strlen
                       (ident_buf) < MIN_IDENT_SIZE) {
                printf("Missing identifier argument, please specify whole "
                       "1024 character or at least 20 characters\n");
                continue;
            }

            remove_package(package_list, ident_buf);
            continue;
        }

        if (strncmp(command_buf, "FETCH ", MAX_COMMAND_SIZE) == 0) {
            char ip_buf[IP_BUFFER_SIZE] = {0};
            int port_buf = 0;
            char space_buf1 = 0;
            char ident_buf[MAX_IDENT_SIZE] = {0};
            char space_buf2 = 0;
            char hash_buf[SHA256_HEX_STRLEN] = {0};
            char space_buf3 = 0;
            uint32_t offset_buf = 0;
            if (sscanf(current_line, "%15[^0-9]%15[^:]:%d%c%1024s%c%64s%c%d",
                       command_buf, ip_buf, &port_buf, &space_buf1,
                       ident_buf, &space_buf2, hash_buf, &space_buf3,
                       &offset_buf) < 7 || strncmp(command_buf, "FETCH ",
                       MAX_COMMAND_SIZE) != 0 || space_buf1 != ' ' ||
                       space_buf2 != ' ') {
                printf("Missing arguments from command\n");
                continue;
            }

            // Look for the peer
            int peer_index;
            if ((peer_index = find_peer(peer_list, ip_buf, port_buf)) == -1) {
                printf("Unable to request chunk, peer not in list\n");
                continue;
            }
            int peer_fd = peer_list->peers[peer_index].peer_fd;

            // Look for the package
            int package_index;
            if ((package_index = find_package(package_list, ident_buf,
                                              MIN_IDENT_SIZE)) == -1) {
                printf("Unable to request chunk, package is not managed\n");
                continue;
            }
            struct bpkg_obj *package = package_list->packages[package_index];

            // Look for the hash in the package
            chunk *target_chunk = get_chunk_from_hash(package, hash_buf, offset_buf);
            if (target_chunk == NULL) {
                printf("Unable to request chunk, chunk hash does not belong to package\n");
                continue;
            }

            // Construct REQ payload
            union btide_payload payload = {0};
            payload.request.file_offset = offset_buf;
            payload.request.data_len = target_chunk->size;
            strncpy(payload.request.chunk_hash, hash_buf, SHA256_HEX_LEN);
            strncpy(payload.request.ident, ident_buf, IDENT_SIZE);

            send_REQ(&payload, peer_fd);

            continue;
        }

        printf("Invalid Input\n");
    }

    free_peer_list(peer_list);
    free_package_list(package_list);
}
