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
        printf("Error Creating Tree\n");
        return  NULL;
    }

    merkle_tree *new_tree = calloc(1, sizeof(merkle_tree));
    new_tree->num_nodes = nhashes + nchunks;
    new_tree->num_inner_nodes = nhashes;
    new_tree->num_leaves = nchunks;
    // Number of nodes at depth d: 2^d, given root node is depth 0
    new_tree->max_depth = (size_t) (log2(nchunks));
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

/**
 * Compute the hash of given data
 * @param data
 * @param data_size
 * @param hash_buf buffer to store the computed hash
 */
void compute_hash(char *data, uint32_t data_size, char *hash_buf) {
    struct sha256_compute_data c_data = {0};
    uint8_t hash_out[SHA256_INT_SZ];
    sha256_compute_data_init(&c_data);
    sha256_update(&c_data, data, data_size);
    sha256_finalize(&c_data, hash_out);
    sha256_output_hex(&c_data, hash_buf);
}

void compute_leaf_hashes(merkle_tree *hashes, char *full_filename) {
    FILE *data_file = fopen(full_filename, "rb");
    if (data_file == NULL) {
        printf("Failed to open the file\n");
        fclose(data_file);
        return;
    }

    for (size_t i = hashes->num_inner_nodes; i < hashes->num_nodes; ++i) {
        merkle_tree_node *current_node = hashes->nodes[i];
        chunk *current_chunk = current_node->value;

        if (fseek(data_file, current_chunk->offset, SEEK_SET) != 0) {
            perror("Failed to offset the file");
            fclose(data_file);
            return;
        }
        char *data_buf = calloc(current_chunk->size, sizeof(char));
        fread(data_buf, sizeof(char), current_chunk->size, data_file);

        // Compute the hash of the current chunk
        compute_hash(data_buf, current_chunk->size,
                     current_node->computed_hash);
        free(data_buf);
    }

    fclose(data_file);
}

void compute_inner_hashes(merkle_tree  *hashes) {
    for (size_t i = hashes->num_inner_nodes; i > 0; --i) {
        merkle_tree_node *current_node = hashes->nodes[i - 1];

        // Concatenate the computed hashes of two children without null
        // terminator
        char *data_buf = calloc(2 * SHA256_HEX_LEN, sizeof(char));
        strncpy(data_buf, current_node->left->computed_hash, SHA256_HEX_LEN);
        for (size_t j = SHA256_HEX_LEN; j < 2 * SHA256_HEX_LEN; ++j) {
            data_buf[j] = current_node->right->computed_hash[j -
                                                             SHA256_HEX_LEN];
        }

        // Compute the hash of the current inner node
        compute_hash(data_buf, 2 * SHA256_HEX_LEN, current_node->computed_hash);
        free(data_buf);
    }
}

/**
 * Check if the computed hash and the expected hash of a node matches
 * @param node
 * @return 1 if matches, 0 otherwise
 */
int compare_node_hash(merkle_tree_node *node) {
    if (node == NULL) {
        return 0;
    }

    if (strcmp(node->expected_hash, node->computed_hash) == 0) {
        return 1;
    }
    return 0;
}

char **get_all_leaf_hashes_from_node(merkle_tree *hashes, merkle_tree_node
*node) {
    // Root node
    if (node->key == 0) {
        // NULL at the end to signal end of leaf hashes
        char **leaf_hashes = calloc(hashes->num_leaves + 1, sizeof(char *));
        for (size_t i = hashes->num_inner_nodes; i < hashes->num_nodes; ++i) {
            leaf_hashes[i - hashes->num_inner_nodes] = calloc
                    (SHA256_HEX_STRLEN, sizeof(char));
            strcpy(leaf_hashes[i - hashes->num_inner_nodes],
                   hashes->nodes[i]->expected_hash);
        }
        return leaf_hashes;
    }

    // Total number of nodes given depth d: 2^(d+1) - 1, given root node is
    // depth 0
    int node_depth = 1;
    int node_depth_offset = 0;
    // Calculates the depth of the node
    for (int d = 1; d <= hashes->max_depth; ++d) {
        int max_index_at_d = (int) pow(2, (d+1)) - 1 - 1;
        if (node->key <= max_index_at_d) {
            node_depth = d;
            // The offset of the node at depth d
            node_depth_offset =  node->key - ((int) pow(2, d) - 1);
            break;
        }
    }

    // Calculates the leaf indices of subtree with the selected node as root
    size_t subtree_num_leaves = (size_t)pow(2, ((int)hashes->max_depth -
    node_depth));
    size_t left_index = hashes->num_inner_nodes + (node_depth_offset *
            subtree_num_leaves);
    size_t right_index = hashes->num_inner_nodes + ((node_depth_offset + 1) *
            subtree_num_leaves);
    // NULL at the end to signal end of leaf hashes
    char **leaf_hashes = calloc(subtree_num_leaves + 1, sizeof(char *));
    for (size_t i = left_index; i < right_index; ++i) {
        leaf_hashes[i - left_index] = calloc(SHA256_HEX_STRLEN, sizeof(char));
        strcpy(leaf_hashes[i - left_index],hashes->nodes[i]->expected_hash);
    }
    return leaf_hashes;
}

/**
 * Check if a node (with child_key) is a descendant of another node (with
 * parent_key)
 * @param parent_key
 * @param child_key
 * @return 1 if true, 0 otherwise
 */
int check_child_from_node(int parent_key, int child_key) {
    // Not possible that child key is smaller than parent key
    if (child_key < parent_key) {
        return 0;
    }
    // Same node
    if (child_key == parent_key) {
        return 1;
    }

    // Relative depth from parent to child
    int rel_d = 1;
    // Left and right indices for the subtree starting from parent node
    int left_index = ((int)pow(2, rel_d) * (parent_key + 1)) - 1;
    int right_index = ((int)pow(2, rel_d) * (parent_key + 2)) - 2;
    while (child_key >= left_index) {
        if (child_key <= right_index) {
            return 1;
        }
        rel_d++;
        left_index = ((int)pow(2, rel_d) * (parent_key + 1)) - 1;
        right_index = ((int)pow(2, rel_d) * (parent_key + 2)) - 2;
    }
    return 0;
}
