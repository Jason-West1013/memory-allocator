#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>

#include "allocator.h"

static uintptr_t allocator(void *(*my_malloc)(size_t)) {
  void *before = sbrk(0);

  void *b1 = my_malloc(10);
  void *b2 = my_malloc(45);
  void *b3 = my_malloc(5);
  void *b4 = my_malloc(13);
  void *b5 = my_malloc(32);
  void *b6 = my_malloc(16);

  my_free(b4);
  my_free(b6);
  my_free(b2);

  void *b7 = my_malloc(10);
  void *b8 = my_malloc(40);

  void *after = sbrk(0);

  my_free(b1);
  my_free(b3);
  my_free(b5);
  my_free(b7);
  my_free(b8);

  return (uintptr_t)after - (uintptr_t)before;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <first-fit|best-fit>\n", argv[0]);
    return 1;
  }
  
  uintptr_t growth = 0;
  if (strcmp(argv[1], "first-fit") == 0) {
    growth = allocator(my_malloc_first_fit); 
  } else if (strcmp(argv[1], "best-fit") == 0) {
    growth = allocator(my_malloc_best_fit);
  } else {
    fprintf(stderr, "unknown strategy: %s\n", argv[1]);
    return 1;
  } 
  printf("Using %s the total heap growth was %lu", argv[1], growth);
}

