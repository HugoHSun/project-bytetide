CC=gcc
CFLAGS=-Wall -std=c2x -g -Wuninitialized -Wvla -Werror -fsanitize=address,leak
LDFLAGS=-lm -lpthread
INCLUDE=-Iinclude

.PHONY: clean

all: pkgmain btide

# Required for Part 1 - Make sure it outputs a .o file
# to either objs/ or ./
# In your directory
sha256.o: src/crypt/sha256.c
	$(CC) -c $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS)

merkletree.o: src/tree/merkletree.c
	$(CC) -c $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS)

pkgchk.o: src/chk/pkgchk.c
	$(CC) -c $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS)

pkgmain: src/pkgmain.c pkgchk.o merkletree.o sha256.o
	$(CC) $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) -o $@

# Required for Part 2 - Make sure it outputs `btide` file
# in your directory ./
config.o: src/config/config.c
	$(CC) -c $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS)

peer.o: src/p2p/peer.c
	$(CC) -c $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS)

package.o: src/p2p/package.c
	$(CC) -c $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS)

p2p_node.o: src/p2p/p2p_node.c
	$(CC) -c $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS)

packet.o: src/net/packet.c
	$(CC) -c $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS)

btide: src/btide.c config.o p2p_node.o peer.o package.o packet.o pkgchk.o merkletree.o sha256.o
	$(CC) $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) -o $@

# Alter your build for p1 tests to build unit-tests for your
# merkle tree, use pkgchk to help with what to test for
# as well as some basic functionality
p1tests:
	bash p1test.sh

# Alter your build for p2 tests to build IO tests
# for your btide client, construct .in/.out files
# and construct a script to help test your client
# You can opt to constructing a program to
# be the tests instead, however please document
# your testing methods
p2tests:
	bash p2test.sh

clean:
	rm -f *.o pkgmain btide
	rm -r tests
