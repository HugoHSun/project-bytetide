#include <stdlib.h>
#include <string.h>

#include "config/config.h"

#define MAX_BTIDE_LINE_SIZE 1024
#define MAX_COMMAND_SIZE 64

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
        sscanf(current_line, "%63s", command_buf);
        if (strncmp(command_buf, "QUIT", 4) == 0) {
            return 0;
        }

    }
}
