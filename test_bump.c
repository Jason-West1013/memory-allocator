#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "allocator.h"

int main(void) {
  size_t test_size = 25;

  // Test 1
  void *p = my_malloc(7);
  assert(((uintptr_t)p % ALIGNMENT) == 0);
  printf("Test 1: Alignment - every returned pointer is a multiple of alignment: {%d}\n\n", ALIGNMENT);

  // Test 2
  void *p1 = my_malloc(10);
  void *p2 = my_malloc(20);
  uintptr_t end1 = (uintptr_t)p1 + 10;
  assert((uintptr_t)p2 >= end1);
  printf("Test 2: Non-overlap - successive allocations don't occupy the same bytes: { %lu, %lu }\n\n", (uintptr_t)p1, (uintptr_t)p2);

  // Test 3
  void *p3 = my_malloc(test_size);

  unsigned char *bytes = (unsigned char *)p3;
  memset(bytes, 0xAB, test_size);

  for (size_t i = 0; i < test_size; i++) {
    assert(bytes[i] == 0xAB);
  }
  printf("Test 3: Writability - full requested size of each block using memtest\n\n");

  // Test 4
  void *p4 = my_malloc(0);
  assert(!p4);
  printf("Test 4: Zero-size - function returns NULL when size is 0\n\n");

  // Test 5
  void *before = sbrk(0);
  void *p5 = my_malloc(test_size);
  void *after = sbrk(0);

  uintptr_t growth = (uintptr_t)after - (uintptr_t)before;
  assert(growth >= test_size);
  assert(growth <= test_size + 128);
  printf("Test 5: Heap Growth - function actually moves heap foward as expected: before - {%lu}, after - {%lu}, malloc return {%lu}\n\n", (uintptr_t)before, (uintptr_t)after, (uintptr_t)p5);

  printf("all tests passed\n");
  return 0;
}
