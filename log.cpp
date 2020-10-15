#include "log.h"

#include <stdio.h>

FILE *log_file = NULL;

int init_logging()
{
    log_file = fopen("dump.log", "w");

    if (log_file == NULL) {
        return 1;
    }

    if (setvbuf(log_file, NULL, _IONBF, 0)) {
        return 1;
    }

    return 0;
}