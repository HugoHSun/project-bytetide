./btide ./config.cfg < $(dirname "$0")/btide_invalid_commands.in | diff $(dirname "$0")/btide_invalid_commands.out -
rm -d tests
