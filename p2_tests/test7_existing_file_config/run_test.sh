touch existing_file.txt
./btide $(dirname "$0")/existing_file.cfg | diff $(dirname "$0")/existing_file_config.out -
rm -f existing_file.txt
