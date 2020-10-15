#ifndef HASH_SUM_H
#define HASH_SUM_H

typedef unsigned int hash_sum_t;

hash_sum_t rol(hash_sum_t hash_sum);
hash_sum_t eval_hash_sum(const unsigned char *buffer, size_t size);

#endif /* HASH_SUM_H */
