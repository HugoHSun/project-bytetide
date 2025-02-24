#include "p2p/package.h"

struct package_list *create_package_list() {
    int init_size = PACKAGES_INIT_SIZE;
    struct package_list *new_list = calloc(1, sizeof(struct package_list));
    new_list->max_size = init_size;
    new_list->num_packages = 0;
    new_list->packages = calloc(init_size, sizeof(struct bpkg_obj *));

    return new_list;
}

void add_package(struct package_list *list, struct bpkg_obj *new_package) {
    // Double the max size when capacity is almost reached
    if ((list->max_size - 1) == list->num_packages) {
        int old_size = list->max_size;
        list->max_size *= 2;
        list->packages = realloc(list->packages, list->max_size * sizeof(struct
                bpkg_obj *));

        for (int i = old_size; i < list->max_size; ++i) {
            list->packages[i] = NULL;
        }
    }

    for (int i = 0; i < list->max_size; ++i) {
        if (list->packages[i] == NULL) {
            list->packages[i] = new_package;
            list->num_packages++;
            return;
        }
    }

    printf("package.c: add_package: ERROR\n");
}

/**
 * Find the package with pkg_ident in the package list
 * @param list
 * @param pkg_ident
 * @param match first n character to match
 * @return the index in the package list, -1 when failed
 */
int find_package(struct package_list *list, char *pkg_ident, int match) {
    for (int i = 0; i < list->max_size; ++i) {
        struct bpkg_obj *current_obj = list->packages[i];
        if (current_obj == NULL) {
            continue;
        }
        if (strncmp(current_obj->ident, pkg_ident, match) == 0) {
            return i;
        }
    }
    return -1;
}

void remove_package(struct package_list *list, char *pkg_ident) {
    int package_i = find_package(list, pkg_ident, 20);
    if (package_i == -1) {
        printf("Identifier provided does not match managed packages\n");
        return;
    }

    bpkg_obj_destroy(list->packages[package_i]);
    list->packages[package_i] = NULL;
    list->num_packages--;
    printf("Package has been removed\n");
}

void print_package(struct bpkg_obj *package, int count) {
    printf("%d. %.32s, %s/%s : ", count, package->ident, package->directory,
    package->filename);
    if (bpkg_complete_check(package)) {
        printf("COMPLETED\n");
    } else {
        printf("INCOMPLETE\n");
    }
}

void print_package_list(struct package_list *list) {
    int print_count = 0;
    for (int i = 0; i < list->max_size; ++i) {
        if (list->packages[i] != NULL) {
            print_count++;
            print_package(list->packages[i], print_count);
        }
    }

    if (print_count == 0) {
        printf("No packages managed\n");
    }

    // Debug check
    if (print_count != list->num_packages) {
        printf("package.c: Did not print all packages\n");
    }
}

void free_package_list(struct package_list *list) {
    if (list == NULL) {
        return;
    }

    for (int i = 0; i < list->max_size; ++i) {
        if (list->packages[i] != NULL) {
            bpkg_obj_destroy(list->packages[i]);
        }
    }
    free(list->packages);
    free(list);
}
