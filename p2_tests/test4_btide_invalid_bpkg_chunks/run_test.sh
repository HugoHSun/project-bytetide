./btide ./config.cfg < $(dirname "$0")/btide_invalid_bpkg_chunks.in | diff $(dirname "$0")/btide_invalid_bpkg_chunks.out -
