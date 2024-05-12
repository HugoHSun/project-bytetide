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
    if (nodes == NULL) {
        perror("Error Creating Tree\n");
        return  NULL;
    }

    merkle_tree *new_tree = calloc(1, sizeof(merkle_tree));
    new_tree->root = nodes[0];
    new_tree->n_nodes = nhashes + nchunks;
    new_tree->nodes = nodes;
    // Linking all the nodes
    for (size_t i = 0; i < nhashes; ++i) {
        size_t left_child = 2 * i + 1;
        size_t right_child = 2 * i + 2;
        nodes[i]->left = nodes[left_child];
        nodes[i]->right = nodes[right_child];
    }
    return new_tree;
}

void free_node(merkle_tree_node *node) {
    if(node->value != NULL) {
        free(node->value);
    }
    free(node);
}

void free_tree(merkle_tree *tree) {
    if(tree->nodes == NULL) {
        return;
    }

    for (size_t i = 0; i < tree->n_nodes; ++i) {
        if (tree->nodes[i] != NULL) {
            free_node(tree->nodes[i]);
        }
    }
    free(tree->nodes);
    free(tree);
}
