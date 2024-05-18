#include "chk/pkgchk.h"

#define FILE_EXIST_MESSAGE "File Exists"
#define FILE_CREATED_MESSAGE "File Created"

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
    char current_line[MAX_LINE_SIZE] = {0};

    // Parsing ident
    if (fgets(current_line, MAX_LINE_SIZE, bpkg_file) == NULL) {
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
    if (fgets(current_line, MAX_LINE_SIZE, bpkg_file) == NULL) {
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
    if (fgets(current_line, MAX_LINE_SIZE, bpkg_file) == NULL) {
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
    if (fgets(current_line, MAX_LINE_SIZE, bpkg_file) == NULL) {
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

    if (fgets(current_line, MAX_LINE_SIZE, bpkg_file) == NULL) {
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
        if (fgets(current_line, MAX_LINE_SIZE, bpkg_file) == NULL) {
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
    if (fgets(current_line, MAX_LINE_SIZE, bpkg_file) == NULL) {
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

    if (fgets(current_line, MAX_LINE_SIZE, bpkg_file) == NULL) {
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
        if (fgets(current_line, MAX_LINE_SIZE, bpkg_file) == NULL) {
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
    if (fopen(bpkg->filename, "r") != NULL) {
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

void compute_chunk_hashes(struct bpkg_obj *bpkg) {
    struct bpkg_query file_query = bpkg_file_check(bpkg);
    if (file_query.hashes == NULL) {
        bpkg_query_destroy(&file_query);
        return;
    }

    char *message = file_query.hashes[0];
    // No hashes to compute if the file doesn't exist
    if (strcmp(message, FILE_CREATED_MESSAGE) == 0) {
        bpkg_query_destroy(&file_query);
        return;
    }
    if (strcmp(message, FILE_EXIST_MESSAGE) != 0) {
        bpkg_query_destroy(&file_query);
        return;
    }
    bpkg_query_destroy(&file_query);

    // Compute the hashes of the leaves
    compute_leaf_hashes(bpkg->hashes, bpkg->filename);
}

/**
 * Retrieves all completed chunks of a package object
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_completed_chunks(struct bpkg_obj *bpkg) {
    compute_chunk_hashes(bpkg);

    size_t qry_size = 0;
    struct bpkg_query qry = {NULL, qry_size};
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
