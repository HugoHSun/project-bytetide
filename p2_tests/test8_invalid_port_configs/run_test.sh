./btide $(dirname "$0")/invalid_port.cfg | diff $(dirname "$0")/invalid_port_config.out -
./btide $(dirname "$0")/invalid_port2.cfg | diff $(dirname "$0")/invalid_port_config.out -
./btide $(dirname "$0")/invalid_port3.cfg | diff $(dirname "$0")/invalid_port_config.out -
rm -d p2tests_dir
