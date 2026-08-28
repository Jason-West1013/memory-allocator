#include <stddef.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "allocator.h"

#define ALIGN_UP(x, align) (((x) + (align) - 1) & ~((uintptr_t)(align) - 1))
#define HEADER_SIZE ALIGN_UP(sizeof(struct header), ALIGNMENT)
#define FOOTER_SIZE ALIGN_UP(sizeof(struct footer), ALIGNMENT)

struct header {
  size_t payload_size;
  struct header *next;
  struct header *prev;
  bool is_free;
};

struct footer {
  size_t payload_size;
};

static char *starting_break = NULL;

static struct header *free_list_head = NULL;

static void add_head_node(struct header *ptr) {
  if (free_list_head == NULL) {
    ptr->next = NULL;
    ptr->prev = NULL;
    free_list_head = ptr;
  } else {
    ptr->prev = NULL;
    ptr->next = free_list_head;
    free_list_head->prev = ptr;
    free_list_head = ptr;
  }
}

static void remove_list_node(struct header *ptr) {
  if (ptr->next == NULL && ptr->prev == NULL) {
    free_list_head = NULL;
  } else if (ptr->prev == NULL) {
    free_list_head = ptr->next;
    ptr->next->prev = NULL;
  } else if (ptr->next == NULL) {
    ptr->prev->next = NULL;
  } else {
    ptr->prev->next = ptr->next;
    ptr->next->prev = ptr->prev;
  }
}

static bool is_valid_free_block(struct header *header, size_t size) {
  return header->is_free && header->payload_size >= size;
}

struct alloc_params {
  void *current_break;
  uintptr_t padding;
  uintptr_t size;
};

static struct alloc_params compute_alloc_params (size_t size) {
  void *current_break = sbrk(0);
  uintptr_t padding = (uintptr_t)(-(intptr_t)current_break) & (ALIGNMENT - 1);

  if (starting_break == NULL) {
    starting_break = (char *)current_break + padding;
  }

  return (struct alloc_params) {
    .current_break = current_break,
    .padding = padding,
    .size = ALIGN_UP(size, ALIGNMENT),
  };
}

static bool is_splittable_block(size_t payload_size, size_t alloc_size) {
  return payload_size - alloc_size >= ALIGNMENT + HEADER_SIZE + FOOTER_SIZE; 
}

static void *free_list_allocation(struct header *found, size_t size) {
  found->is_free = false;
  remove_list_node(found);

  if (is_splittable_block(found->payload_size, size)) {
    size_t unclaimed_payload_size = found->payload_size - size - HEADER_SIZE - FOOTER_SIZE;
    found->payload_size = size;

    struct footer *claimed_footer = (struct footer *)((char *)found + HEADER_SIZE + size);
    claimed_footer->payload_size = size;

    struct header *unclaimed_header = (struct header *)((char *)claimed_footer + FOOTER_SIZE);
    unclaimed_header->is_free = true;
    unclaimed_header->payload_size = unclaimed_payload_size;

    struct footer *unclaimed_footer = (struct footer *)((char *)unclaimed_header + HEADER_SIZE + unclaimed_header->payload_size);
    unclaimed_footer->payload_size = unclaimed_payload_size;

    add_head_node(unclaimed_header);
  }

  return (char *)found + HEADER_SIZE;
}

static void *create_block(struct alloc_params params) {
  uintptr_t total_allocation = params.padding + params.size + HEADER_SIZE + FOOTER_SIZE;
  void *result = sbrk((intptr_t)total_allocation);

  if (result == (void *)-1) {
    return NULL;
  } 

  struct header *block_header = (struct header *)((char *)params.current_break + params.padding);
  block_header->payload_size = params.size;
  block_header->is_free = false;
  block_header->next = NULL;
  block_header->prev = NULL;

  struct footer *block_footer = (struct footer *)((char *)params.current_break + params.padding + HEADER_SIZE + params.size);
  block_footer->payload_size = params.size;

  return (char *)params.current_break + params.padding + HEADER_SIZE;
}

void *my_malloc_first_fit(size_t size) {
  if (size == 0) {
    return NULL;
  }

  struct alloc_params params = compute_alloc_params(size);
  
  struct header *current_header = free_list_head;
  while (current_header != NULL) {
    if (is_valid_free_block(current_header, params.size)) {
      return free_list_allocation(current_header, params.size);
    }
    current_header = current_header->next;
  }

  void *block = create_block(params);

  if (block == NULL) {
    return NULL;
  }

  return block;
}

static bool is_best_fit_candidate(struct header *candidate, struct header *best_fit) {
  return best_fit == NULL || best_fit->payload_size > candidate->payload_size;
}

void *my_malloc_best_fit(size_t size) {
  if (size == 0) {
    return NULL;
  }

  struct alloc_params params = compute_alloc_params(size); 

  struct header *best_fit_header = NULL;
  struct header *candidate_header = free_list_head;
  while (candidate_header != NULL) {
    if (is_valid_free_block(candidate_header, params.size) && is_best_fit_candidate(candidate_header, best_fit_header)) {
      best_fit_header = candidate_header;
    } 
    candidate_header = candidate_header->next;
  }

  if (best_fit_header != NULL) {
    return free_list_allocation(best_fit_header, params.size);
  }

  void *block = create_block(params);

  if (block == NULL) {
    return NULL;
  }

  return block;
}

void my_free(void *ptr) { 
  if (ptr == NULL) {
    return;
  }

  struct header *freed_header = (struct header *)((char *)ptr - HEADER_SIZE);
  freed_header->is_free = true;

  struct header *start_header = freed_header;
  struct footer *end_footer = (struct footer *)((char *)freed_header + HEADER_SIZE + freed_header->payload_size);
  size_t total_payload_size = freed_header->payload_size;

  struct header *search_header = freed_header;
  while ((char *)search_header != starting_break && search_header->is_free) {
    struct footer *prev_footer = (struct footer *)((char *)search_header - FOOTER_SIZE);
    struct header *prev_header = (struct header *)((char *)prev_footer - prev_footer->payload_size - HEADER_SIZE);

    if (prev_header->is_free) {
      remove_list_node(prev_header);
      start_header = prev_header;
      total_payload_size = total_payload_size + prev_header->payload_size + FOOTER_SIZE + HEADER_SIZE;
    } 
    search_header = prev_header;
  }
  
  search_header = freed_header;
  while ((char *)search_header != sbrk(0) && search_header->is_free) {
    struct header *next_header = (struct header *)((char *)search_header + HEADER_SIZE + search_header->payload_size + FOOTER_SIZE);

   if ((char *)next_header != sbrk(0) && next_header->is_free) {
      remove_list_node(next_header);
      end_footer = (struct footer*)((char *)next_header + HEADER_SIZE + next_header->payload_size);
      total_payload_size = total_payload_size + FOOTER_SIZE + HEADER_SIZE + next_header->payload_size;
    }
    search_header = next_header;
  }

  start_header->payload_size = end_footer->payload_size = total_payload_size;
  add_head_node(start_header);
}

