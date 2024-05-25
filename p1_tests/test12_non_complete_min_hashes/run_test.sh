./pkgmain $(dirname "$0")/test.bpkg -min_hashes | diff $(dirname "$0")/non_complete_min_hashes.out -
