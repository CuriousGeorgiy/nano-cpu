#include "canary.h"

#include "assert.h"

canary_t left_canary(const void *ptr)
{
    return *(canary_t *)(((char *) ptr) - CANARY_SIZE);
}

canary_t right_canary(const void *ptr, size_t size)
{
    return *(canary_t *)(((char *) ptr) + size);
}