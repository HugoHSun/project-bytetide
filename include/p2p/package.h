#ifndef PACKAGE_H
#define PACKAGE_H

#include "chk/pkgchk.h"

#define PACKAGES_INIT_SIZE 8

struct package_list {
    int max_size;
    int num_packages;
    struct bpkg_obj **packages;
};

struct package_list *create_package_list();

void add_package(struct package_list *list, struct bpkg_obj *new_package);

/**
 * Find the package with pkg_ident in the package list
 * @param list
 * @param pkg_ident
 * @param match
 * @return the index in the package list, -1 when failed
 */
int find_package(struct package_list *list, char *pkg_ident, int match);

/**
 * Find the chunk hash in a package
 * @param package
 * @param hash
 * @param offset
 * @return the chunk size if found, 0 otherwise
 */
uint32_t find_hash_in_package(struct bpkg_obj *package, char
*hash, uint32_t offset);

void remove_package(struct package_list *list, char *pkg_ident);

void print_package_list(struct package_list *list);

void free_package_list(struct package_list *list);

#endif