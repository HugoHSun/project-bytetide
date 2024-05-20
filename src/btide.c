#include <stdlib.h>
#include <string.h>

#include "config/config.h"

#define MAX_BTIDE_LINE_SIZE 5521
#define MAX_COMMAND_SIZE 16

//
// PART 2
//

int main(int argc, char **argv) {
    // Load the configuration file
    struct config config = {0};
    int result = parse_config(argv[1], &config);
    if (result != 0) {
        return result;
    }

    // Command line interface
    char current_line[MAX_BTIDE_LINE_SIZE] = {0};
    char command_buf[MAX_COMMAND_SIZE] = {0};
    while (fgets(current_line, MAX_BTIDE_LINE_SIZE, stdin) != NULL) {
        sscanf(current_line, "%15s", command_buf);

        if (strncmp(command_buf, "QUIT", MAX_COMMAND_SIZE) == 0) {
            return 0;
        }

        if (strncmp(command_buf, "PACKAGES", MAX_COMMAND_SIZE) == 0) {
            printf("PACKAGES COMMAND\n");
            continue;
        }

        if (strncmp(command_buf, "PEERS", MAX_COMMAND_SIZE) == 0) {
            printf("PEERS COMMAND\n");
            continue;
        }

        if (strncmp(command_buf, "CONNECT", MAX_COMMAND_SIZE) == 0) {
            printf("CONNECT COMMAND\n");
            continue;
        }

        if (strncmp(command_buf, "DISCONNECT", MAX_COMMAND_SIZE) == 0) {
            printf("DISCONNECT COMMAND\n");
            continue;
        }

        if (strncmp(command_buf, "ADDPACKAGE", MAX_COMMAND_SIZE) == 0) {
            printf("ADDPACKAGE COMMAND\n");
            continue;
        }

        if (strncmp(command_buf, "REMPACKAGE", MAX_COMMAND_SIZE) == 0) {
            printf("REMPACKAGE COMMAND\n");
            continue;
        }

        if (strncmp(command_buf, "FETCH", MAX_COMMAND_SIZE) == 0) {
            printf("FETCH COMMAND\n");
            continue;
        }

        printf("Invalid Input\n");
    }
}
