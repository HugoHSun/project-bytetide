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
    if (sscanf(current_line, "directory:%4096s", config->directory) != 1) {
        return INVALID_FIELD;
    }
    DIR *dp = opendir(config->directory);
    if (dp == NULL) {
        // Parsed directory is a file
        FILE *fp = fopen(config->directory, "r");
        if (fp != NULL) {
            fclose(fp);
            return INVALID_DIRECTORY;
        }

        mode_t mode = S_IRWXU;
        if (mkdir(config->directory, mode) == -1) {
            return INVALID_DIRECTORY;
        }
    }

    // Parsing max_peers
    if (fgets(current_line, MAX_CONFIG_LINE_SIZE, config_file) == NULL) {
        closedir(dp);
        return INVALID_FIELD;
    }
    if (sscanf(current_line, "max_peers:%d", &config->max_peers) != 1) {
        closedir(dp);
        return INVALID_FIELD;
    }
    if (config->max_peers < MIN_PEER_NUM || config->max_peers > MAX_PEER_NUM) {
        closedir(dp);
        return INVALID_PEER_NUM;
    }

    // Parsing port
    if (fgets(current_line, MAX_CONFIG_LINE_SIZE, config_file) == NULL) {
        closedir(dp);
        return INVALID_FIELD;
    }
    if (sscanf(current_line, "port:%d", &config->max_peers) != 1) {
        closedir(dp);
        return INVALID_FIELD;
    }
    if (config->max_peers < MIN_PORT_NUM || config->max_peers > MAX_PORT_NUM) {
        closedir(dp);
        return INVALID_PORT_NUM;
    }
    config->port = (u_int16_t)config->max_peers;

    closedir(dp);
    return 0;
}
