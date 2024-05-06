#include <stdlib.h>
#include <stdio.h>

#include "../include/chk/pkgchk.h"

#define MAX_LINE_SIZE 1048

// PART 1


/**
 * Loads the package for when a valid path is given
 */
struct bpkg_obj *bpkg_load(const char *path) {
    FILE *bpkg_file = fopen(path, "r");
    if (bpkg_file == NULL) {
        perror("Invalid Package Path");
        return NULL;
    }

    struct bpkg_obj *obj = calloc(1, sizeof(struct bpkg_obj));
    char current_line[MAX_LINE_SIZE];

    // Parsing ident
    if (fgets(current_line, MAX_LINE_SIZE, bpkg_file) == NULL) {
        perror("Error Parsing Package File: ident");
        free(obj);
        return NULL;
    }
    char space_buf = 0;
    if (sscanf(current_line, "ident:%c%s", &space_buf, obj->ident) != 2 ||
        space_buf != ' ') {
        perror("Invalid 'ident' in Package File");
        free(obj);
        return NULL;
    }

    // Parsing filename
    if (fgets(current_line, MAX_LINE_SIZE, bpkg_file) == NULL) {
        perror("Error Parsing Package File: filename");
        free(obj);
        return NULL;
    }
    if (sscanf(current_line, "filename:%c%s", &space_buf, obj->filename) != 2 ||
        space_buf != ' ') {
        perror("Invalid 'filename' in Package File");
        free(obj);
        return NULL;
    }

    // Parsing size
    if (fgets(current_line, MAX_LINE_SIZE, bpkg_file) == NULL) {
        perror("Error Parsing Package File: size");
        free(obj);
        return NULL;
    }
    if (sscanf(current_line, "size:%c%u", &space_buf, &(obj->size)) != 2 ||
        space_buf != ' ') {
        perror("Invalid 'size' in Package File");
        free(obj);
        return NULL;
    }

    // Parsing nhashes
    if (fgets(current_line, MAX_LINE_SIZE, bpkg_file) == NULL) {
        perror("Error Parsing Package File: nhashes");
        free(obj);
        return NULL;
    }
    if (sscanf(current_line, "nhashes:%c%u", &space_buf, &(obj->nhashes)) !=
        2 ||
        space_buf != ' ') {
        perror("Invalid 'nhashes' in Package File");
        free(obj);
        return NULL;
    }

    // Parsing hashes
    obj->hashes = calloc(obj->nhashes, sizeof(char *));
    for (uint32_t i = 0; i < obj->nhashes; ++i) {
        if (fgets(current_line, MAX_LINE_SIZE, bpkg_file) == NULL) {
            perror("Error Parsing Package File: hashes");
            bpkg_obj_destroy(obj);
            return NULL;
        }
        obj->hashes[i] = calloc(HASH_SIZE, sizeof(char));
        if (sscanf(current_line, "%s", obj->hashes[i]) != 1) {
            perror("Invalid 'hash' in Package File");
            bpkg_obj_destroy(obj);
            return NULL;
        }
    }

    // Parsing nchunks
    if (fgets(current_line, MAX_LINE_SIZE, bpkg_file) == NULL) {
        perror("Error Parsing Package File: nchunks");
        bpkg_obj_destroy(obj);
        return NULL;
    }
    if (sscanf(current_line, "nchunks:%c%u", &space_buf, &(obj->nchunks)) !=
        2 || space_buf != ' ') {
        perror("Invalid 'nchunks' in Package File");
        bpkg_obj_destroy(obj);
        return NULL;
    }

    // Parsing chunks
    obj->chunks = calloc(obj->nchunks, sizeof(struct chunk *));
    for (uint32_t i = 0; i < obj->nchunks; ++i) {
        if (fgets(current_line, MAX_LINE_SIZE, bpkg_file) == NULL) {
            perror("Error Parsing Package File: chunks");
            bpkg_obj_destroy(obj);
            return NULL;
        }
        obj->chunks[i] = malloc(sizeof(struct chunk));
        if (sscanf(current_line, "%s,%u,%u", obj->chunks[i]->hash,
                   &(obj->chunks[i]->offset), &(obj->chunks[i]->size)) != 3) {
            perror("Invalid 'chunk' in Package File");
            bpkg_obj_destroy(obj);
            return NULL;
        }
    }

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
        query.hashes[0] = "File Exists";
    } else {
        fopen(bpkg->filename, "w");
        query.hashes[0] = "File Created";
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
    struct bpkg_query qry = {0};

    return qry;
}

/**
 * Retrieves all completed chunks of a package object
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_completed_chunks(struct bpkg_obj *bpkg) {
    struct bpkg_query qry = {0};
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
    free(qry->hashes);
}

/**
 * Deallocates memory at the end of the program,
 * make sure it has been completely deallocated
 */
void bpkg_obj_destroy(struct bpkg_obj *obj) {
    uint32_t nhashes = 0;
    uint32_t nchunks = 0;

    nhashes = obj->nhashes;
    for (int i = 0; i < nhashes; ++i) {
        if (obj->hashes[i] != NULL) {
            free(obj->hashes[i]);
        }
    }
    free(obj->hashes);

    nchunks = obj->nchunks;
    for (int i = 0; i < nchunks; ++i) {
        if (obj->chunks[i] != NULL) {
            free(obj->chunks[i]);
        }
    }
    free(obj->chunks);

    free(obj);
}


