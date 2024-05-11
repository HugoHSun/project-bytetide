#include "tree/merkletree.h"

chunk *create_chunk(uint32_t offset, uint32_t size) {
    chunk *new_chunk = calloc(1, sizeof(chunk));

    new_chunk->offset = offset;
    new_chunk->size = size;

    return new_chunk;
}

merkle_tree_node *create_node(int key, chunk *value, char
        *expected_hash) {
    merkle_tree_node *new_node = calloc(1, sizeof(merkle_tree_node));

    new_node->key = key;
    new_node->value = value;
    strncpy(new_node->expected_hash, expected_hash, SHA256_HEXLEN);
    new_node->left = NULL;
    new_node->right = NULL;

    return new_node;
}

merkle_tree *create_tree(merkle_tree_node **nodes, uint32_t nhashes, uint32_t
nchunks) {
    // Linking all the nodes
    for (size_t i = 0; i < nhashes; ++i) {
        size_t left_index = 2 * i + 1;
        size_t right_index = 2 * i + 2;
        nodes[i]->left = nodes[left_index];
        nodes[i]->right = nodes[right_index];
    }
    struct merkle_tree *new_tree = calloc(1, sizeof(struct merkle_tree));
    new_tree->root = nodes[0];
    new_tree->n_nodes = nhashes + nchunks;

    return new_tree;
}

void free_node(merkle_tree_node *node) {
    if(node->value != NULL) {
        free(node->value);
    }
    free(node);
}

void free_tree(merkle_tree *tree) {

}
