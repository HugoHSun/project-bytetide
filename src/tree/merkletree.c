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
    strncpy(new_node->expected_hash, expected_hash, SHA256_HEX_STRLEN);
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
    new_tree->num_nodes = nhashes + nchunks;
    new_tree->num_inner_nodes = nhashes;
    new_tree->num_leaves = nchunks;
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

    for (size_t i = 0; i < tree->num_nodes; ++i) {
        if (tree->nodes[i] != NULL) {
            free_node(tree->nodes[i]);
        }
    }
    free(tree->nodes);
    free(tree);
}

void compute_leaf_hashes(merkle_tree *hashes, char *filename) {
    FILE *data_file = fopen(filename, "rb");
    if (data_file == NULL) {
        printf("Failed to open the file\n");
        fclose(data_file);
        return;
    }

    for (size_t i = hashes->num_inner_nodes; i < hashes->num_nodes; ++i) {
        merkle_tree_node *current_node = hashes->nodes[i];
        chunk *current_chunk = current_node->value;

        if (fseek(data_file, current_chunk->offset, SEEK_SET) != 0) {
            printf("Failed to offset the file\n");
            fclose(data_file);
            return;
        }
        char *data_buf = calloc(current_chunk->size, sizeof(char));
        fread(data_buf, sizeof(char), current_chunk->size, data_file);

        // Compute the hash of the current chunk
        struct sha256_compute_data c_data = {0};
        uint8_t hash_out[SHA256_INT_SZ];
        sha256_compute_data_init(&c_data);
        sha256_update(&c_data, data_buf, current_chunk->size);
        sha256_finalize(&c_data, hash_out);
        sha256_output_hex(&c_data, current_node->computed_hash);

        free(data_buf);
    }

    fclose(data_file);
}

int compare_node_hash(merkle_tree_node *node) {
    if (node == NULL) {
        return 0;
    }

    if (strcmp(node->expected_hash, node->computed_hash) == 0) {
        return 1;
    }
    return 0;
}
