#include "chk/pkgchk.h"

void free_node_buf(merkle_tree_node **buf, uint32_t size) {
    if (buf == NULL) {
        return;
    }

    for (int i = 0; i < size; ++i) {
        if (buf[i] != NULL) {
            free_node(buf[i]);
        }
    }
    free(buf);
}


/**
 * Loads the package for when a valid path is given
 */
struct bpkg_obj *bpkg_load(const char *path) {
    FILE *bpkg_file = fopen(path, "r");
    if (bpkg_file == NULL) {
        printf("Invalid Path to Package File\n");
        return NULL;
    }

    struct bpkg_obj *obj = calloc(1, sizeof(struct bpkg_obj));
    char current_line[MAX_BPKG_LINE_SIZE] = {0};

    // Parsing ident
    if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
        printf("Missing Field in Package File: ident\n");
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }
    if (sscanf(current_line, "ident:%1024s", obj->ident) != 1) {
        printf("Invalid Field in Package File: ident\n");
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }

    // Parsing filename
    if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
        printf("Missing Field in Package File: filename\n");
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }
    if (sscanf(current_line, "filename:%256s", obj->filename) != 1) {
        printf("Invalid Field in Package File: filename\n");
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }

    // Parsing size
    if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
        printf("Missing Field in Package File: size\n");
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }
    if (sscanf(current_line, "size:%u", &(obj->size)) != 1) {
        printf("Invalid Field in Package File: size\n");
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }

    // Parsing nhashes
    if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
        printf("Missing Field in Package File: nhashes\n");
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }
    if (sscanf(current_line, "nhashes:%u", &(obj->nhashes)) != 1) {
        printf("Invalid Field in Package File: nhashes\n");
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }

    if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
        printf("Invalid format in Package File: missing 'hashes:'\n");
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }
    char str_buf[FILENAME_MAX] = {0};
    if (sscanf(current_line, "%s", str_buf) != 1 || !strcmp(str_buf,
                                                            "hashes:\n")) {
        printf("Invalid format in Package File: missing 'hashes:'\n");
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }

    // Parsing non-leaf nodes into a buffer
    merkle_tree_node **all_nodes = calloc(obj->nhashes, sizeof
            (merkle_tree_node *));
    char hash_buf[SHA256_HEX_STRLEN] = {0};
    char tab_buf = 0;
    for (int i = 0; i < obj->nhashes; ++i) {
        if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
            printf("Missing Field in Package File: hashes\n");
            fclose(bpkg_file);
            free_node_buf(all_nodes, obj->nhashes);
            free(obj);
            return NULL;
        }
        if (sscanf(current_line, "%1c%64s", &tab_buf, hash_buf) != 2 ||
        tab_buf != '\t') {
            printf("Invalid Field in Package File: hashes\n");
            fclose(bpkg_file);
            free_node_buf(all_nodes, obj->nhashes);
            free(obj);
            return NULL;
        }
        all_nodes[i] = create_node(i, NULL, hash_buf);
    }

    // Parsing nchunks
    if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
        printf("Missing Field in Package File: nchunks\n");
        fclose(bpkg_file);
        free_node_buf(all_nodes, obj->nhashes);
        free(obj);
        return NULL;
    }
    if (sscanf(current_line, "nchunks:%u", &(obj->nchunks)) != 1) {
        printf("Invalid Field in Package File: nchunks\n");
        fclose(bpkg_file);
        free_node_buf(all_nodes, obj->nhashes);
        free(obj);
        return NULL;
    }

    if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
        printf("Invalid format in Package File: missing 'chunks:'\n");
        fclose(bpkg_file);
        free_node_buf(all_nodes, obj->nhashes);
        free(obj);
        return NULL;
    }
    if (sscanf(current_line, "%s", str_buf) != 1 || !strcmp(str_buf,
                                                            "chunks:\n")) {
        printf("Invalid format in Package File: missing 'chunks:'\n");
        fclose(bpkg_file);
        free_node_buf(all_nodes, obj->nhashes);
        free(obj);
        return NULL;
    }

    // Parsing leaf nodes into a buffer
    size_t num_nodes = obj->nhashes + obj->nchunks;
    size_t new_buf_size = num_nodes * sizeof(merkle_tree_node *);
    // Reallocate a larger memory to include the leaf nodes
    all_nodes = realloc(all_nodes, new_buf_size);
    for (size_t i = obj->nhashes; i < num_nodes; ++i) {
        all_nodes[i] = NULL;
    }
    uint32_t offset_buf = 0;
    uint32_t size_buf = 0;
    for (int i = 0; i < obj->nchunks; ++i) {
        if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
            printf("Invalid Field in Package File: chunks\n");
            fclose(bpkg_file);
            free_node_buf(all_nodes, num_nodes);
            free(obj);
            return NULL;
        }
        if (sscanf(current_line, "%1c%64s,%u,%u", &tab_buf, hash_buf,
                   &offset_buf, &size_buf) != 4 || tab_buf != '\t') {
            printf("Invalid Field in Package File: chunks\n");
            fclose(bpkg_file);
            free_node_buf(all_nodes, num_nodes);
            free(obj);
            return NULL;
        }
        int index = obj->nhashes + i;
        chunk *new_chunk = create_chunk(offset_buf, size_buf);
        all_nodes[index] = create_node(index, new_chunk, hash_buf);
    }

    merkle_tree *new_tree = create_tree(all_nodes, obj->nhashes, obj->nchunks);
    obj->hashes = new_tree;
    fclose(bpkg_file);
    return obj;
}


/**
 * Loads the package for when a valid path is given, with no error messages
 * printed
 */
struct bpkg_obj *bpkg_load_no_message(const char *path, char *directory) {
    FILE *bpkg_file = fopen(path, "r");
    if (bpkg_file == NULL) {
        return NULL;
    }

    struct bpkg_obj *obj = calloc(1, sizeof(struct bpkg_obj));
    char current_line[MAX_BPKG_LINE_SIZE] = {0};

    // Parsing ident
    if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }
    if (sscanf(current_line, "ident:%1024s", obj->ident) != 1) {
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }

    strncpy(obj->directory, directory, MAX_DATA_DIRECTORY_SIZE);
    // Parsing filename
    if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }
    if (sscanf(current_line, "filename:%256s", obj->filename) != 1) {
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }

    // Parsing size
    if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }
    if (sscanf(current_line, "size:%u", &(obj->size)) != 1) {
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }

    // Parsing nhashes
    if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }
    if (sscanf(current_line, "nhashes:%u", &(obj->nhashes)) != 1) {
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }

    if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }
    char str_buf[FILENAME_MAX] = {0};
    if (sscanf(current_line, "%s", str_buf) != 1 || !strcmp(str_buf,
                                                            "hashes:\n")) {
        fclose(bpkg_file);
        free(obj);
        return NULL;
    }

    // Parsing non-leaf nodes into a buffer
    merkle_tree_node **all_nodes = calloc(obj->nhashes, sizeof
            (merkle_tree_node *));
    char hash_buf[SHA256_HEX_STRLEN] = {0};
    char tab_buf = 0;
    for (int i = 0; i < obj->nhashes; ++i) {
        if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
            fclose(bpkg_file);
            free_node_buf(all_nodes, obj->nhashes);
            free(obj);
            return NULL;
        }
        if (sscanf(current_line, "%1c%64s", &tab_buf, hash_buf) != 2 ||
            tab_buf != '\t') {
            fclose(bpkg_file);
            free_node_buf(all_nodes, obj->nhashes);
            free(obj);
            return NULL;
        }
        all_nodes[i] = create_node(i, NULL, hash_buf);
    }

    // Parsing nchunks
    if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
        fclose(bpkg_file);
        free_node_buf(all_nodes, obj->nhashes);
        free(obj);
        return NULL;
    }
    if (sscanf(current_line, "nchunks:%u", &(obj->nchunks)) != 1) {
        fclose(bpkg_file);
        free_node_buf(all_nodes, obj->nhashes);
        free(obj);
        return NULL;
    }

    if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
        fclose(bpkg_file);
        free_node_buf(all_nodes, obj->nhashes);
        free(obj);
        return NULL;
    }
    if (sscanf(current_line, "%s", str_buf) != 1 || !strcmp(str_buf,
                                                            "chunks:\n")) {
        fclose(bpkg_file);
        free_node_buf(all_nodes, obj->nhashes);
        free(obj);
        return NULL;
    }

    // Parsing leaf nodes into a buffer
    size_t num_nodes = obj->nhashes + obj->nchunks;
    size_t new_buf_size = num_nodes * sizeof(merkle_tree_node *);
    // Reallocate a larger memory to include the leaf nodes
    all_nodes = realloc(all_nodes, new_buf_size);
    for (size_t i = obj->nhashes; i < num_nodes; ++i) {
        all_nodes[i] = NULL;
    }
    uint32_t offset_buf = 0;
    uint32_t size_buf = 0;
    for (int i = 0; i < obj->nchunks; ++i) {
        if (fgets(current_line, MAX_BPKG_LINE_SIZE, bpkg_file) == NULL) {
            fclose(bpkg_file);
            free_node_buf(all_nodes, num_nodes);
            free(obj);
            return NULL;
        }
        if (sscanf(current_line, "%1c%64s,%u,%u", &tab_buf, hash_buf,
                   &offset_buf, &size_buf) != 4 || tab_buf != '\t') {
            fclose(bpkg_file);
            free_node_buf(all_nodes, num_nodes);
            free(obj);
            return NULL;
        }
        int index = obj->nhashes + i;
        chunk *new_chunk = create_chunk(offset_buf, size_buf);
        all_nodes[index] = create_node(index, new_chunk, hash_buf);
    }

    merkle_tree *new_tree = create_tree(all_nodes, obj->nhashes, obj->nchunks);
    obj->hashes = new_tree;
    fclose(bpkg_file);
    return obj;
}


void get_file_full_path(char *full_filename, struct bpkg_obj *bpkg) {
    if (bpkg->directory[0] != '\0') {
        strncpy(full_filename, bpkg->directory, MAX_DATA_DIRECTORY_SIZE - 1);
        strcat(full_filename, "/");
        strncat(full_filename, bpkg->filename, MAX_FILENAME_SIZE - 1);
    } else {
        strcpy(full_filename, bpkg->filename);
    }
}


int check_file_existence(char *filename) {
    FILE * fp = fopen(filename, "r");
    if (fp != NULL) {
        fclose(fp);
        return 1;
    }
    return 0;
}


int get_data(struct bpkg_obj *obj, uint32_t size, uint32_t abs_offset,
        char
        *data_buf) {
    char full_path[MAX_DATA_DIRECTORY_SIZE + MAX_FILENAME_SIZE];
    get_file_full_path(full_path, obj);
    // The file in bpkg does not exist
    if (check_file_existence(full_path) == 0) {
        return 0;
    }

    FILE *fp = fopen(full_path, "rb");
    if (fseek(fp, abs_offset, SEEK_SET) != 0) {
        perror("Failed to offset the file");
        fclose(fp);
        return 0;
    }
    if (fread(data_buf, sizeof(char), size, fp) < size) {
        perror("get_data:fread:");
    }
    fclose(fp);
    return 1;
}


int write_data(struct bpkg_obj *obj, uint16_t size, uint32_t abs_offset, char
*data_buf) {
    char full_path[MAX_DATA_DIRECTORY_SIZE + MAX_FILENAME_SIZE];
    get_file_full_path(full_path, obj);

    FILE *fp = NULL;
    if (check_file_existence(full_path) == 0) {
        if ((fp = fopen(full_path, "wb")) == NULL) {
            perror("write_data - wb:");
        }
        // Create file with specified size
        char null_byte = 0;
        for (size_t i = 0; i < obj->size; ++i) {
            fwrite(&null_byte, sizeof(char), 1, fp);
        }
        fseek(fp, 0, SEEK_SET);
    } else {
        if ((fp = fopen(full_path, "r+b")) == NULL) {
            perror("write_data - r+b:");
        }
        // Make sure the file size is correct
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        char null_byte = 0;
        for (long i = file_size; i < obj->size; ++i) {
            fwrite(&null_byte, sizeof(char), 1, fp);
        }
        fseek(fp, 0, SEEK_SET);
    }

    // printf("WRITING DATA: size: %u offset: %u\n\n", size, abs_offset);
    if (fseek(fp, abs_offset, SEEK_SET) != 0) {
        perror("Failed to offset the file");
        fclose(fp);
        return 0;
    }

    if (fwrite(data_buf, sizeof(char), size, fp) < size) {
        perror("write_data:fwrite:");
    }
    fclose(fp);
    return 1;
}


/**
 * Checks to see if the referenced filename in the bpkg file
 * exists or not.
 * @param bpkg, constructed bpkg object
 * @return query_result, a single string should be
 *      printable in hashes with len sized to 1.
 * 		If the file exists, hashes[0] should contain "File Exists"
 *		If the file does not exist, hashes[0] should contain "File Created"
 */
struct bpkg_query bpkg_file_check(struct bpkg_obj *bpkg) {
    struct bpkg_query query = {NULL, 1};
    if (bpkg == NULL) {
        return query;
    }

    query.hashes = calloc(1, sizeof(char *));
    if (check_file_existence(bpkg->filename)) {
        query.hashes[0] = calloc(strlen(FILE_EXIST_MESSAGE) + 1, sizeof(char));
        strcpy(query.hashes[0], FILE_EXIST_MESSAGE);
    } else {
        // Create a new file with specified byte size, initialised with NULLs
        FILE *new_file = fopen(bpkg->filename, "wb");
        char null_byte = 0;
        for (size_t i = 0; i < bpkg->size; ++i) {
            fwrite(&null_byte, sizeof(char), 1, new_file);
        }
        fclose(new_file);
        query.hashes[0] = calloc(strlen(FILE_CREATED_MESSAGE) + 1, sizeof(char));
        strcpy(query.hashes[0], FILE_CREATED_MESSAGE);
    }

    return query;
}


/**
 * Retrieves a list of all hashes within the package/tree
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_all_hashes(struct bpkg_obj *bpkg) {
    struct bpkg_query qry = {NULL, bpkg->hashes->num_nodes};

    qry.hashes = calloc(bpkg->hashes->num_nodes, sizeof(char *));
    for (size_t i = 0; i < bpkg->hashes->num_nodes; ++i) {
        qry.hashes[i] = calloc(SHA256_HEX_STRLEN, sizeof(char));
        strcpy(qry.hashes[i], bpkg->hashes->nodes[i]->expected_hash);
    }

    return qry;
}


int compute_chunk_hashes(struct bpkg_obj *bpkg) {
    if (bpkg == NULL) {
        return 0;
    }

    char full_filename[MAX_FILENAME_SIZE + MAX_DATA_DIRECTORY_SIZE];
    get_file_full_path(full_filename, bpkg);

    // No hashes to compute if the file doesn't exist
    if (check_file_existence(full_filename) == 0) {
        return 0;
    }

    // Compute the hashes of the leaves
    compute_leaf_hashes(bpkg->hashes, full_filename);
    return 1;
}


/**
 * Check if a file is complete in the package
 */
int bpkg_complete_check(struct bpkg_obj *bpkg) {
    if (compute_chunk_hashes(bpkg) == 0) {
        return 0;
    }

    merkle_tree *hashes = bpkg->hashes;
    for (size_t i = hashes->num_inner_nodes; i < hashes->num_nodes; ++i) {
        if (!compare_node_hash(hashes->nodes[i])) {
            return 0;
        }
    }

    return 1;
}


/**
 * Check if a chunk hash is in the package, return the chunk size if found
 */
uint32_t bpkg_chunk_hash_check(struct bpkg_obj *bpkg, char *hash, uint32_t
        offset) {
    merkle_tree *hashes = bpkg->hashes;
    // Iterate through all leaf nodes
    for (size_t i = hashes->num_inner_nodes; i < hashes->num_nodes; ++i) {
        struct merkle_tree_node *current_node = hashes->nodes[i];
        if (strcmp(current_node->expected_hash, hash) == 0) {
            // No offset specified
            if (offset == 0) {
                return current_node->value->size;
            }
            // Offset does not match, continue searching
            if (offset < current_node->value->offset || offset >=
            (current_node->value->offset + current_node->value->size)) {
                continue;
            } else {
                return current_node->value->size;
            }
        }
    }

    return 0;
}


uint32_t get_offset_from_hash(struct bpkg_obj *bpkg, char *hash) {
    merkle_tree *hashes = bpkg->hashes;
    // Iterate through all leaf nodes
    for (size_t i = hashes->num_inner_nodes; i < hashes->num_nodes; ++i) {
        struct merkle_tree_node *current_node = hashes->nodes[i];
        if (strcmp(current_node->expected_hash, hash) == 0) {
                return current_node->value->offset;
        }
    }

    return 0;
}


/**
 * Retrieves all completed chunks of a package object
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_completed_chunks(struct bpkg_obj *bpkg) {
    struct bpkg_query qry = {0};
    // The file doesn't exist hence no completed chunks
    if (compute_chunk_hashes(bpkg) == 0) {
        return qry;
    }

    size_t qry_size = 0;
    qry.hashes = calloc(qry_size, sizeof(char *));
    merkle_tree *hashes = bpkg->hashes;
    for (size_t i = hashes->num_inner_nodes; i < hashes->num_nodes; ++i) {
        if (compare_node_hash(hashes->nodes[i])) {
            qry_size++;
            qry.hashes = realloc(qry.hashes, qry_size * sizeof(char *));
            qry.hashes[qry_size - 1] = calloc(SHA256_HEX_STRLEN, sizeof(char));
            strcpy(qry.hashes[qry_size - 1], hashes->nodes[i]->expected_hash);
        }
    }

    qry.len = qry_size;
    return qry;
}


void compute_all_hashes(struct bpkg_obj *bpkg) {
    // No need to compute inner hashes when no leaf hashes are computed
    if (compute_chunk_hashes(bpkg) == 0) {
        return;
    }

    // Compute the hashes of the inner nodes
    compute_inner_hashes(bpkg->hashes);
}


/**
 * Gets only the required/min hashes to represent the current completion state
 * Return the smallest set of hashes of completed branches to represent
 * the completion state of the file.
 *
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_min_completed_hashes(struct bpkg_obj *bpkg) {
    struct bpkg_query qry = {0};
    compute_all_hashes(bpkg);
    merkle_tree *hashes = bpkg->hashes;

    // The root node is complete
    if (compare_node_hash(hashes->nodes[0])) {
        qry.len = 1;
        qry.hashes = calloc(qry.len, sizeof(char *));
        qry.hashes[0] = calloc(SHA256_HEX_STRLEN, sizeof(char));
        strcpy(qry.hashes[0], hashes->nodes[0]->expected_hash);
        return qry;
    }

    size_t qry_size = 0;
    qry.hashes = calloc(qry_size, sizeof(char *));
    int *added_keys = calloc(qry_size, sizeof(int));
    // BFS on every node
    for (size_t i = 1 ; i < hashes->num_nodes; ++i) {
        merkle_tree_node *current_node = hashes->nodes[i];
        if (compare_node_hash(current_node)) {
            // Check if the node is a descent of any existing hashes
            int add = 1;
            for (size_t j = 0;  j < qry_size; ++j) {
                if (check_child_from_node(added_keys[j], current_node->key)) {
                    add = 0;
                    break;
                }
            }
            if (add) {
                qry_size++;
                qry.hashes = realloc(qry.hashes, qry_size * sizeof(char *));
                qry.hashes[qry_size - 1] = calloc(SHA256_HEX_STRLEN, sizeof(char));
                strcpy(qry.hashes[qry_size - 1], current_node->expected_hash);

                added_keys = realloc(added_keys, qry_size * sizeof(int));
                added_keys[qry_size - 1] = current_node->key;
            }
        }
    }

    free(added_keys);
    qry.len = qry_size;
    return qry;
}


/**
 * Retrieves all chunk hashes given a certain an ancestor hash (or itself)
 * Example: If the root hash was given, all chunk hashes will be outputted
 * 	If the root's left child hash was given, all chunks corresponding to
 * 	the first half of the file will be outputted
 * 	If the root's right child hash was given, all chunks corresponding to
 * 	the second half of the file will be outputted
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_all_chunk_hashes_from_hash(struct bpkg_obj *bpkg,
                                                      char *hash) {
    struct bpkg_query qry = {0};

    merkle_tree *hashes = bpkg->hashes;
    merkle_tree_node *current_node = NULL;
    for (size_t i = 0; i < hashes->num_nodes; ++i) {
        current_node = hashes->nodes[i];
        if (strncmp(current_node->expected_hash, hash, SHA256_HEX_LEN) == 0) {
            break;
        }
    }
    // The given hash is not in the merkle tree
    if (current_node == NULL) {
        return qry;
        // The given hash is a leaf node
    } else if (current_node->value != NULL) {
        qry.len = 1;
        qry.hashes = calloc(qry.len, sizeof(char *));
        qry.hashes[0] = calloc(SHA256_HEX_STRLEN, sizeof(char));
        strncpy(qry.hashes[0], hash, SHA256_HEX_LEN);
        return qry;
    }

    // Get all leaf hashes, with NULL at the end of the array
    char** leaf_hashes = get_all_leaf_hashes_from_node(hashes, current_node);
    size_t qry_size = 0;
    qry.hashes = calloc(qry_size, sizeof(char *));
    while (leaf_hashes[qry_size] != NULL) {
        qry_size++;
        qry.hashes = realloc(qry.hashes, qry_size * sizeof(char *));
        qry.hashes[qry_size - 1] = calloc(SHA256_HEX_STRLEN, sizeof(char));
        strcpy(qry.hashes[qry_size - 1], leaf_hashes[qry_size - 1]);
    }
    qry.len = qry_size;
    // Free the memory of leaf_hashes
    for (size_t i = 0; i < qry_size; ++i) {
        if (leaf_hashes[i] != NULL) {
            free(leaf_hashes[i]);
        }
    }
    free(leaf_hashes);

    return qry;
}


/**
 * Deallocates the query result after it has been constructed from
 * the relevant queries above.
 */
void bpkg_query_destroy(struct bpkg_query *qry) {
    if (qry == NULL || qry->hashes == NULL) {
        return;
    }

    for (size_t i = 0; i < qry->len; ++i) {
        if (qry->hashes[i] != NULL) {
            free(qry->hashes[i]);
        }
    }
    free(qry->hashes);
}

/**
 * Deallocates memory at the end of the program,
 * make sure it has been completely deallocated
 */
void bpkg_obj_destroy(struct bpkg_obj *obj) {
    if (obj == NULL) {
        return;
    }

    if (obj->hashes != NULL) {
        free_tree(obj->hashes);
    }
    free(obj);
}
