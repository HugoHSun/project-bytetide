./btide $(dirname "$0")/config.cfg < $(dirname "$0")/more_package_commands.in | diff $(dirname "$0")/more_package_commands.out -
rm -r p2tests_dir
