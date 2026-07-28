#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "allocator.h"

#define ALIGN_UP(x, align) (((x) + (align) - 1) & ~((uintptr_t)(align) - 1))
#define HEADER_SIZE ALIGN_UP(sizeof(struct header), ALIGNMENT)

struct header {
  size_t payload_size;
  bool is_free;
  struct header *next;
};

static struct header *free_list_head = NULL;

void *my_malloc(size_t size) {
  if (size == 0) {
    return NULL;
  }
  
  void *current_break = sbrk(0);

  uintptr_t addr_padding = (uintptr_t)(-(intptr_t)current_break) & (ALIGNMENT - 1);
  uintptr_t rounded_up_size = ALIGN_UP(size, ALIGNMENT); 

  struct header *prev_header = NULL;
  struct header *current_header = free_list_head;
  while (current_header != NULL) {
    if (current_header->is_free && current_header->payload_size >= rounded_up_size) {
      
      struct header *found = current_header;
      found->is_free = false;

      if (prev_header == NULL) {
        free_list_head = current_header->next;
      } else { 
        prev_header->next = current_header->next;
      }

      return (char *)found + HEADER_SIZE;
    }
    prev_header = current_header;
    current_header = current_header->next;

  }

  uintptr_t total_allocation = addr_padding + rounded_up_size + HEADER_SIZE;
  void *result = sbrk((intptr_t)total_allocation);

  if (result == (void *)-1) {
    return NULL;
  }

  struct header *block_header = (struct header *)((char *)current_break + addr_padding);
  block_header->payload_size = rounded_up_size;
  block_header->is_free = false;
  block_header->next = NULL;

  return (char *)current_break + addr_padding + HEADER_SIZE;
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

