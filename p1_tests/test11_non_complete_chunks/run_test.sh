./pkgmain $(dirname "$0")/test2.bpkg -chunk_check | diff $(dirname "$0")/non_complete_chunks.out -
