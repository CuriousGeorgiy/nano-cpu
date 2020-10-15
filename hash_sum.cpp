#include "hash_sum.h"

hash_sum_t rol(hash_sum_t hash_sum)
{
    return  hash_sum << 1 | hash_sum >> (8 * sizeof(hash_sum_t) - 1);
}

hash_sum_t eval_hash_sum(const unsigned char *buffer, size_t size)
{
    hash_sum_t hash_sum = 0;

    for (size_t i = 0; i < size; ++i) {
        hash_sum_t a = buffer[i];
        hash_sum = rol(hash_sum) + buffer[i];
    }

    return hash_sum;
}

