./btide ./config.cfg < $(dirname "$0")/btide_missing_file.in | diff $(dirname "$0")/btide_missing_file.out -
