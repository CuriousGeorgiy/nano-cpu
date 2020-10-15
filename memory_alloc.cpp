#include "memory_alloc.h"

#include "canary.h"
#include "error.h"

#include <stdlib.h>

void *calloc_with_border_canaries(size_t num, size_t size)
{
    char *buffer = (char *) calloc(1, CANARY_SIZE + num * size + CANARY_SIZE);

    if (buffer == NULL) {
        ERROR_OCCURRED_CALLING(calloc, "returned NULL");
        return NULL;
    }

    *((canary_t *) buffer) = *((canary_t *) (buffer + CANARY_SIZE + num * size)) = CANARY_VALUE;

    return buffer + CANARY_SIZE;
}

void *realloc_with_border_canaries(void *ptr, size_t new_size)
{
    char *buffer = (char *) realloc((char *) ptr - CANARY_SIZE, CANARY_SIZE + new_size + CANARY_SIZE);

    if (buffer == NULL) {
        ERROR_OCCURRED_CALLING(calloc, "returned NULL");
        return NULL;
    }

    *((canary_t *) buffer) = *((canary_t *) (buffer + CANARY_SIZE + new_size)) = CANARY_VALUE;

    return buffer + CANARY_SIZE;
}
