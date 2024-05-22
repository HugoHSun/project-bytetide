#ifndef PACKAGE_H
#define PACKAGE_H

#include "chk/pkgchk.h"

struct package_list {
    int max_size;
    int num_packages;
    struct bpkg_obj **packages;
};

struct package_list *create_package_list();

void add_package(struct package_list *list, struct bpkg_obj *new_package);

void remove_package(struct package_list *list, char *pkg_ident);

void print_package_list(struct package_list *list);

void free_package_list(struct package_list *list);

#endif