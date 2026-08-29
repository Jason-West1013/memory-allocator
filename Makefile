CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g -D_DEFAULT_SOURCE -fsanitize=address,undefined

tests: tests.c allocator.c
	$(CC) $(CFLAGS) -o $@ $^
benchmark: benchmark.c allocator.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f tests benchmark
