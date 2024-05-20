#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_CONFIG_LINE_SIZE 5012
#define MAX_DIRECTORY_SIZE 4097
#define MIN_PEER_NUM 1
#define MAX_PEER_NUM 2048
#define MIN_PORT_NUM 1025
#define MAX_PORT_NUM 65535

// Error codes
#define INVALID_CONFIG 1
#define INVALID_FIELD 2
#define INVALID_DIRECTORY 3
#define INVALID_PEER_NUM 4
#define INVALID_PORT_NUM 5

struct config {
    int max_peers;
    u_int16_t port;
};

int parse_config(char *filename, struct config *config);

#endif
