./pkgmain $(dirname "$0")/DOESNOTEXIST.in -file_check | diff $(dirname "$0")/package_main_nonexistent_file.out -
