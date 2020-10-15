#ifndef CANARY_H
#define CANARY_H

typedef unsigned long long canary_t;

enum canary {
    CANARY_SIZE = sizeof(canary_t),
    CANARY_VALUE = 0xDEADBEDull
};

canary_t left_canary(const void *ptr);
canary_t right_canary(const void *ptr, size_t size);

#endif /* CANARY_H */
