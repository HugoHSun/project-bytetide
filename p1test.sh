TEST_DIR="./p1_tests"

chmod u+x btide
echo "PART 1 TESTS: "
for dir in $(ls -d ${TEST_DIR}/*/); do
  echo "**************************************************"
  echo "Running test: $dir"
  chmod u+x $dir/run_test.sh
  $dir/run_test.sh
  echo **************************************************$'\n'
done
