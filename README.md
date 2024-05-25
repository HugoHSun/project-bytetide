# COMP2017 Assignment 3 - Project ByteTide
## Semester 1, 2024


## General
- All the header files are in the `include` directory, where they are 
  further divided into folders. Every header file has a respective C file in 
  the `src` directory. 
- All the C code files are in the `src` directory. The `pkgmain.c` 
  file is the Command Line Interface of part 1. The `btide.c`
  file is the Command Line Interface of part 2. 
- Run `make all` to build both parts of the project,  or `make pkgmain` for 
  part 1 or `make btide` for part 2.
- All binaries and object files in the root directory can be removed using 
  `make clean`


## Part 1 - ByteTide Package Loader & Merkle Tree
### Organisation
- `src/crypt/sha256.c`: used for compute hashes on data.
- `src/tree/merkletree.c`: implementation of the merkle tree, which is 
  implemented using tree as an array. This file includes the data structure of 
  merkle tree and helper functions such as computing hashes and tree 
  operations. The key of each node represents its index in the array. The root 
  node is the first element in the array, and linear probing on the array 
  represents level-order traversal. For a given node with index `i`, its 
  left child's index is calculated by `2i+1` and right 
  child index is `2i+2`. 
- `src/chk/pkgchk.c`: implementation of the bpkg data structure, and helper 
  functions for bpkg operations. 
- `src/pkgmain.c`: command line interface that utilises functions in `pkgchk.c`.


## Part 2 - Configuration, Networking and Program
### Organisation
- `src/config/config.c`: used for parsing configuration files when starting 
  the btide application.
- `src/p2p/peer.c`: implements the underlying data structure (dynamic array) 
  and helper functions for managing peers in the btide application.
- `src/p2p/package.c`: implements the underlying data structure (dynamic array)
  and helper functions for managing packages in the btide application.
- `src/p2p/p2p_node.c`: includes thread functions for initialising 
  connection requests, acting as a server and handling any packets. 
  Responsible for request listening and chunk handling. 
- `src/net/packet.c`: implements the data structure of network packets 
  and payloads, including helper functions for sending and receiving packets. 
- `src/btide.c`: the command line interface of the btide application, 
  utilises all the above C files, `pkgchk` for bpkg helper functions and 
  `merkletree.c` for packet data integrity check. Responsible for handling 
  commands and sending packets. 

### How to run
- Run `make btide` and then `./btide <config file>`


## Tests
### Organisation
- All the part 1 test cases are located in the `p1_tests` directory. 
- All the part 2 test cases are located in the `p2_tests` directory.
- Each folder represents a test case, and it includes all the input files, output files
  and the test script required for the test case.
- The folder name represents the situation that is being tested by the test 
  case. 
- The tests cases are ran by main test scripts in the root directory: 
  `p1test.sh` and `p2test.sh`, which run Part 1 testcases and Part 2 
  testcases respectively. 

### How to run
#### Run in the terminal
1. Make sure the project is built by using `make all` in the project root 
   directory
2. Run the shell command `chmod u+x p1test.sh p2test.sh` to add execution permission for the test scripts
2. Run the shell command `./p1test.sh` for part 1 tests and `./p2test.sh` 
   for part 2 tests. 
3. Any differences in the actual outputs and expected outputs will be 
   printed to the terminal. 

#### Run using make
- Build the project using `make all`
- Part 1 tests: `make p1tests`
- Part 2 tests: `make p2tests`
