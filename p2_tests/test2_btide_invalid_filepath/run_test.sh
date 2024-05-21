./btide ./config.cfg < $(dirname "$0")/btide_invalid_filepath.in | diff $(dirname "$0")/btide_invalid_filepath.out -
