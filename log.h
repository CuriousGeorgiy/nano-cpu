#ifndef LOG_H
#define LOG_H

#include <stdio.h>

extern FILE *log_file;

int init_logging();

#define logging(fmt, ...) do {                                     \
                              fprintf(log_file, fmt, __VA_ARGS__); \
                              fflush(log_file);                    \
                          } while (0)
#endif /* LOG_H */
