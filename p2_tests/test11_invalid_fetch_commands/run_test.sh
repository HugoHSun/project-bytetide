{
  sleep 1
  echo QUIT
} | ./btide $(dirname "$0")/config1.cfg

./btide $(dirname "$0")/config2.cfg < $(dirname "$0")/invalid_fetch_commands.in | diff $(dirname "$0")/invalid_fetch_commands.out -

rm -r p2tests_dir
