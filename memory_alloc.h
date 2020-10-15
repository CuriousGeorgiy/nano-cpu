#ifndef MEMORY_ALLOC_H
#define MEMORY_ALLOC_H

#include "canary.h"

#define FREE(ptr) do {              \
                      free(ptr);    \
                      (ptr) = NULL; \
                  } while (0)

#define FREE_WITH_CANARY_BORDER(ptr) do { \
                                        free((char *) (ptr) - CANARY_SIZE); \
                                        (ptr) = NULL; \
                                     } while (0)

void *calloc_with_border_canaries(size_t num, size_t size);

void *realloc_with_border_canaries(void *ptr, size_t new_size);

#endif /* MEMORY_ALLOC_H */
