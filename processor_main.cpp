#include <stdlib.h>
#include <assert.h>

#include "error.h"
#include "file.h"
#include "memory_alloc.h"
#include "processor.h"

char *read_assembly_file_to_buffer(FILE *input);

int main(int argc, const char *argv[])
{
    --argc;

    if (init_logging()) {
        return EXIT_FAILURE;
    }

    if (argc == 0 || argc > 1) {
        printf("Specify input file\n");
        return EXIT_FAILURE;
    }

    const char *input_file_name = argv[1];

    FILE *input = fopen(input_file_name, "rb");

    if (input == NULL) {
        ERROR_OCCURRED_CALLING(fopen, "returned NULL");
        printf("Failed to execute input file, check dump.log for more information\n");
        return EXIT_FAILURE;
    }
    
    char *buffer = read_assembly_file_to_buffer(input);
    
    if (buffer == NULL) {
        ERROR_OCCURRED_CALLING(read_assebmly_file_to_buffer, "returned NULL");
        printf("Failed to execute input file, check dump.log for more information\n");
        return EXIT_FAILURE;
    }

    size_t assembly_size = *(size_t *)(buffer + sizeof(short) + sizeof(char));
    Processor processor;
    
    if (processor_ctor(&processor, buffer + sizeof(short) + sizeof(char) + sizeof(size_t), assembly_size)) {
        ERROR_OCCURRED_CALLING(processor_ctor, "returned nonzero value");
        printf("Failed to execute input file, check dump.log for more information\n");
        return EXIT_FAILURE;
    }

    if (processor_execute_assembly(&processor)) {
        ERROR_OCCURRED_CALLING(processor_execute_assembly, "returned nonzero value");
        printf("Failed to execute input file, check dump.log for more information\n");
        return EXIT_FAILURE;
    }

    printf("Successfully executed input file\n");

    processor_dtor(&processor);
    FREE(buffer);

    return EXIT_SUCCESS;
}

char *read_assembly_file_to_buffer(FILE *input)
{
    assert(input != NULL);
    
    long filesize = file_size(input);

    if (filesize <= 0) {
        ERROR_OCCURRED_CALLING(file_size, "returned non-positive value");
        return NULL;
    }

    char *buffer = (char *) calloc(filesize, sizeof(*buffer));

    if (buffer == NULL) {
        ERROR_OCCURRED_CALLING(calloc, "returned NULL");
        return NULL;
    }

    if (fread(buffer, sizeof(*buffer), filesize, input) != filesize) {
        ERROR_OCCURRED_CALLING(fread, "incorrect number of bytes read");
        return NULL;
    }
    
    return buffer;
}