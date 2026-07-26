#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "allocator.h"

#define ALIGN_UP(x, align) (((x) + (align) - 1) & ~((uintptr_t)(align) - 1))
#define HEADER_SIZE ALIGN_UP(sizeof(struct header), ALIGNMENT)

struct header {
  size_t size;
  bool is_free;
  struct header *next;
};

static struct header *free_list_head = NULL;

void *my_malloc(size_t size) {
  if (size == 0) {
    return NULL;
  }

  void *start_addr = sbrk(0);

  uintptr_t padding = (uintptr_t)(-(intptr_t)start_addr) & (ALIGNMENT - 1);
  uintptr_t rounded_size = ALIGN_UP(size, ALIGNMENT); 

  struct header *free_list_prev = NULL;
  struct header *free_list = free_list_head;
  while (free_list != NULL) {
    if (free_list->is_free && free_list->size >= rounded_size) {
      
      struct header *found = free_list;
      found->is_free = false;

      if (free_list_prev == NULL) {
        free_list_head = free_list->next;
      } else { 
        free_list_prev->next = free_list->next;
      }

      return (char *)found + HEADER_SIZE;
    }
    free_list_prev = free_list;
    free_list = free_list->next;

  }

  uintptr_t total = padding + rounded_size + HEADER_SIZE;
  void *new_addr = sbrk((intptr_t)total);

  if (new_addr == (void *)-1) {
    return NULL;
  }

  struct header *new_header = (struct header *)((char *)start_addr + padding);
  new_header->size = rounded_size;
  new_header->is_free = false;
  new_header->next = NULL;

  return (char *)start_addr + padding + HEADER_SIZE;
}

void my_free(void *ptr) {
  if (ptr == NULL) {
    return;
  }

  struct header *freed_header = (struct header *)((char *)ptr - HEADER_SIZE);
  freed_header->is_free = true;
  freed_header->next = free_list_head;
  free_list_head = freed_header;
}

