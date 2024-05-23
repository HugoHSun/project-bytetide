./btide ./config.cfg < $(dirname "$0")/invalid_connect.in | diff $(dirname "$0")/invalid_connect.out -
