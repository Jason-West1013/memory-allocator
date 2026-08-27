#ifndef ALLOCATOR_H
#define ALLOCATOR_H
#include <stddef.h>

#define ALIGNMENT 16

void *my_malloc(size_t size);
void *my_malloc_best_fit(size_t size);
void my_free(void *ptr);

#endif
