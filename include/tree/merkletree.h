#ifndef MERKLE_TREE_H
#define MERKLE_TREE_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>

#include "crypt/sha256.h"

#define SHA256_HEX_LEN 64
#define SHA256_HEX_STRLEN (SHA256_HEX_LEN + 1)

typedef struct chunk {
    uint32_t offset;
    uint32_t size;
} chunk;

typedef struct merkle_tree_node {
    int key;
    chunk *value; // NULL for non-leaf nodes
    struct merkle_tree_node *left;
    struct merkle_tree_node *right;
    char expected_hash[SHA256_HEX_STRLEN];
    char computed_hash[SHA256_HEX_STRLEN];
} merkle_tree_node;

typedef struct merkle_tree {
    size_t num_nodes;
    size_t num_inner_nodes;
    size_t num_leaves;
    size_t max_depth;
    merkle_tree_node **nodes;
} merkle_tree;

chunk *create_chunk(uint32_t offset, uint32_t size);

merkle_tree_node *create_node(int key, chunk *value, char
        *expected_hash);

merkle_tree *create_tree(merkle_tree_node **nodes, uint32_t nhashes, uint32_t
nchunks);

void free_node(merkle_tree_node *node);

void free_tree(merkle_tree *tree);

/**
 * Compute the hash of given data
 * @param data
 * @param data_size
 * @param hash_buf buffer to store the computed hash
 */
void compute_hash(char *data, uint32_t data_size, char *hash_buf);

void compute_leaf_hashes(merkle_tree  *hashes, char *full_filename);

void compute_inner_hashes(merkle_tree  *hashes);

/**
 * Check if the computed hash and the expected hash of a node matches
 * @param node
 * @return 1 if matches, 0 otherwise
 */
int compare_node_hash(merkle_tree_node *node);

char **get_all_leaf_hashes_from_node(merkle_tree *hashes, merkle_tree_node
*node);

/**
 * Check if a node (with child_key) is a descendant of another node (with
 * parent_key)
 * @param parent_key
 * @param child_key
 * @return 1 if true, 0 otherwise
 */
int check_child_from_node(int parent_key, int child_key);

#endif
