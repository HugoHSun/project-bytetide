#include <string.h>

#include "chk/pkgchk.h"
#include "p2p/peer.h"

#define MAX_BTIDE_LINE_SIZE 5521
#define MAX_COMMAND_SIZE 16
#define IP_BUFFER_SIZE 16

//
// PART 2
//

int main(int argc, char **argv) {
    // Load the configuration file
    struct config config = {0};
    int result = parse_config(argv[1], &config);
    if (result != 0) {
        printf("btide: Failed to read configuration\n");
        return result;
    }

    // Start the server in a new thread
    pthread_t server_thread;
    if (pthread_create(&server_thread, NULL, start_server, &config) != 0) {
        printf("btide: Failed to start server\n");
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
            return 0;
        }

        if (strncmp(command_buf, "PACKAGES", MAX_COMMAND_SIZE) == 0 && (strlen
        (current_line) == 8 || strlen(current_line) == 9)) {
            printf("PACKAGES COMMAND\n");
            continue;
        }

        if (strncmp(command_buf, "PEERS", MAX_COMMAND_SIZE) == 0 && (strlen
        (current_line) == 5 || strlen(current_line) == 6)) {
            printf("PEERS COMMAND\n");
            continue;
        }

        // Append a space at the end for strcmp
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

            printf("COMMAND: %s|| IP: %s|| PORT: %d||\n", command_buf,
                   ip_buf, port_buf);

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

            printf("COMMAND: %s|| IP: %s|| PORT: %d||\n", command_buf,
                   ip_buf, port_buf);

            continue;
        }

        if (strncmp(command_buf, "ADDPACKAGE ", MAX_COMMAND_SIZE) == 0) {
            char space_buf = 0;
            char filname_buf[MAX_FILENAME_SIZE] = {0};
            if (sscanf(current_line, "%15s%c%256c", command_buf, &space_buf,
                       filname_buf) != 3 || space_buf != ' ') {
                printf("Missing file argument\n");
                continue;
            }

            printf("ADDPACKAGE COMMAND Filename: %s\n", filname_buf);
            continue;
        }

        if (strncmp(command_buf, "REMPACKAGE ", MAX_COMMAND_SIZE) == 0) {
            char space_buf = 0;
            char ident_buf[MAX_IDENT_SIZE] = {0};
            if (sscanf(current_line, "%15s%c%1024s", command_buf, &space_buf,
                       ident_buf) != 3 || space_buf != ' ' || strlen
                       (ident_buf) < 20) {
                printf("Missing identifier argument, please specify whole "
                       "1024 character or at least 20 characters\n");
                continue;
            }

            printf("REMPACKAGE COMMAND\n");
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
            int offset_buf = -1;
            if (sscanf(current_line, "%15[^0-9]%15[^:]:%d%c%1024s%c%64s%c%d",
                       command_buf, ip_buf, &port_buf, &space_buf1,
                       ident_buf, &space_buf2, hash_buf, &space_buf3,
                       &offset_buf) < 7 || strncmp(command_buf, "FETCH ",
                       MAX_COMMAND_SIZE) != 0 || space_buf1 != ' ' ||
                       space_buf2 != ' ') {
                printf("Missing arguments from command\n");
                continue;
            }
            if (offset_buf != -1) {
                printf("OFFSET: %d", offset_buf);
            }

            printf("FETCH COMMAND\n");
            continue;
        }

        printf("Invalid Input\n");
    }
}
