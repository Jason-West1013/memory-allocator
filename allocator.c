#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "allocator.h"

void *my_malloc(size_t size) {
  if (size == 0) {
    return NULL;
  }

  void *start_addr = sbrk(0);
  char *start_addr_bytes = (char *)start_addr;

  uintptr_t padding = (uintptr_t)(-(intptr_t)start_addr) & (ALIGNMENT - 1);
  uintptr_t rounded_size = (size + ALIGNMENT - 1) & ~(uintptr_t)(ALIGNMENT - 1);
  uintptr_t total = padding + rounded_size;

  void *new_addr = sbrk((intptr_t)total);

  if (new_addr == (void *)-1) {
    return NULL;
  }

  return start_addr_bytes + padding;
}

