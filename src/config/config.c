#include "config/config.h"

int parse_config(char *filename, struct config *config) {
    FILE *config_file = fopen(filename, "r");
    if (config_file == NULL) {
        return INVALID_CONFIG;
    }
    char current_line[MAX_CONFIG_LINE_SIZE] = {0};

    // Parsing directory
    if (fgets(current_line, MAX_CONFIG_LINE_SIZE, config_file) == NULL) {
        return INVALID_FIELD;
    }
    char directory[MAX_DIRECTORY_SIZE] = {0};
    if (sscanf(current_line, "directory:%4096s", directory) != 1) {
        return INVALID_FIELD;
    }
    DIR *dp = opendir(directory);
    if (dp == NULL) {
        // Parsed directory is a file
        FILE *fp = fopen(directory, "r");
        if (fp != NULL) {
            fclose(fp);
            return INVALID_DIRECTORY;
        }

        mode_t mode = S_IRWXU;
        if (mkdir(directory, mode) == -1) {
            return INVALID_DIRECTORY;
        }
    }

    // Parsing max_peers
    if (fgets(current_line, MAX_CONFIG_LINE_SIZE, config_file) == NULL) {
        closedir(dp);
        return INVALID_FIELD;
    }
    int int_buf = 0;
    if (sscanf(current_line, "max_peers:%d", &int_buf) != 1) {
        closedir(dp);
        return INVALID_FIELD;
    }
    if (int_buf < MIN_PEER_NUM || int_buf > MAX_PEER_NUM) {
        closedir(dp);
        return INVALID_PEER_NUM;
    }
    config->max_peers = int_buf;

    // Parsing port
    if (fgets(current_line, MAX_CONFIG_LINE_SIZE, config_file) == NULL) {
        closedir(dp);
        return INVALID_FIELD;
    }
    int_buf = 0;
    if (sscanf(current_line, "port:%d", &int_buf) != 1) {
        closedir(dp);
        return INVALID_FIELD;
    }
    if (int_buf < MIN_PORT_NUM || int_buf > MAX_PORT_NUM) {
        closedir(dp);
        return INVALID_PORT_NUM;
    }
    config->port = (u_int16_t)int_buf;

    closedir(dp);
    return 0;
}
