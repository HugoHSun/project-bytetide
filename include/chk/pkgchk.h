#ifndef PKGCHK_H
#define PKGCHK_H

#include "tree/merkletree.h"

#define FILE_EXIST_MESSAGE "File Exists"
#define FILE_CREATED_MESSAGE "File Created"

// Have extra byte for the null terminator
#define MAX_IDENT_SIZE 1025
#define MAX_DATA_DIRECTORY_SIZE 4097
#define MAX_FILENAME_SIZE 257
#define MAX_BPKG_LINE_SIZE 1048

/**
 * Query object, allows you to assign
 * hash strings to it.
 * Typically: malloc N number of strings for hashes
 *    after malloc the space for each string
 *    Make sure you deallocate in the destroy function
 */
struct bpkg_query {
    char **hashes;
    size_t len;
};

struct bpkg_obj {
    char ident[MAX_IDENT_SIZE];
    char directory[MAX_DATA_DIRECTORY_SIZE];
    char filename[MAX_FILENAME_SIZE];
    uint32_t size;
    uint32_t nhashes;
    uint32_t nchunks;
    struct merkle_tree *hashes;
};

/**
 * Loads the package for when a value path is given
 */
struct bpkg_obj *bpkg_load(const char *path);

/**
 * Loads the package for when a valid path is given, with no error messages
 * printed
 */
struct bpkg_obj *bpkg_load_no_message(const char *path, char *directory);

void get_file_full_path(char *full_filename, struct bpkg_obj *obj);

int check_file_existence(char *filename);

int get_data(struct bpkg_obj *obj, uint32_t size, uint32_t abs_offset,
        char
        *data_buf);

int write_data(struct bpkg_obj *obj, int size, uint32_t abs_offset, char
        *data_buf);

/**
 * Checks to see if the referenced filename in the bpkg file
 * exists or not.
 * @param bpkg, constructed bpkg object
 * @return query_result, a single string should be
 *      printable in hashes with len sized to 1.
 * 		If the file exists, hashes[0] should contain "File Exists"
 *		If the file does not exist, hashes[0] should contain "File Created"
 */
struct bpkg_query bpkg_file_check(struct bpkg_obj *bpkg);

/**
 * Retrieves a list of all hashes within the package/tree
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_all_hashes(struct bpkg_obj *bpkg);

/**
 * Check if a file is complete in the package
 */
int bpkg_complete_check(struct bpkg_obj *bpkg);

/**
 * Check if a chunk hash is in the package, return the chunk size if found
 */
uint32_t bpkg_chunk_hash_check(struct bpkg_obj *bpkg, char *hash, uint32_t
        offset);

/**
 * Retrieves all completed chunks of a package object
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_completed_chunks(struct bpkg_obj *bpkg);


/**
 * Gets the mininum of hashes to represented the current completion state
 * Example: If chunks representing start to mid have been completed but
 * 	mid to end have not been, then we will have (N_CHUNKS/2) + 1 hashes
 * 	outputted
 *
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_min_completed_hashes(struct bpkg_obj *bpkg);


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
struct bpkg_query
bpkg_get_all_chunk_hashes_from_hash(struct bpkg_obj *bpkg, char *hash);


/**
 * Deallocates the query result after it has been constructed from
 * the relevant queries above.
 */
void bpkg_query_destroy(struct bpkg_query *qry);

/**
 * Deallocates memory at the end of the program,
 * make sure it has been completely deallocated
 */
void bpkg_obj_destroy(struct bpkg_obj *obj);

#endif

