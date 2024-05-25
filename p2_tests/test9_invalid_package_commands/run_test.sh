touch invalid.bpkg
./btide $(dirname "$0")/config.cfg < $(dirname "$0")/invalid_package_commands.in | diff $(dirname "$0")/invalid_package_commands.out -
rm -d p2tests_dir
rm -f invalid.bpkg
