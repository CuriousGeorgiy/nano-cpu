#include "file.h"

#include "error.h"

#include <cassert>

long countFileSize(std::FILE *file)
{
    assert(file != NULL);

    if (fseek(file, 0, SEEK_END)) {
        ERROR_OCCURRED_CALLING(fseek, "returned nonzero value");
        return -1;
    }

    long file_size = ftell(file);
    rewind(file);

    return file_size;
}
