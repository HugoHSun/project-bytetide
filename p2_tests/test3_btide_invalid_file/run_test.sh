./btide ./config.cfg < $(dirname "$0")/btide_invalid_file.in | diff $(dirname "$0")/btide_invalid_file.out -
