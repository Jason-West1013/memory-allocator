CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g -D_DEFAULT_SOURCE -fsanitize=address,undefined

app: test_bump.c allocator.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f app test test_bump
