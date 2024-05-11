#ifndef MERKLE_TREE_H
#define MERKLE_TREE_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "crypt/sha256.h"

#define SHA256_HEXLEN (64)

typedef struct chunk {
    uint32_t offset;
    uint32_t size;
} chunk;

typedef struct merkle_tree_node {
    int key;
    chunk *value; // NULL for non-leaf nodes
    struct merkle_tree_node *left;
    struct merkle_tree_node *right;
    char expected_hash[SHA256_HEXLEN];
    char computed_hash[SHA256_HEXLEN];
} merkle_tree_node;

typedef struct merkle_tree {
    struct merkle_tree_node *root;
    size_t n_nodes;
} merkle_tree;

chunk *create_chunk(uint32_t offset, uint32_t size);

merkle_tree_node *create_node(int key, chunk *value, char
        *expected_hash);

merkle_tree *create_tree(merkle_tree_node **nodes, uint32_t nhashes, uint32_t
nchunks);

void free_node(merkle_tree_node *node);

void free_tree(merkle_tree *tree);

#endif
