CFLAGS := -Wall -Wextra -pedantic -std=c2x

build/main: src/main.c | build
	$(CC) $(CFLAGS) $^ -o $@

build: ; @mkdir build

what-CC: ; @realpath `which $(CC)`
