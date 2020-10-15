#include <stdlib.h>

#include "assembler.h"
#include "error.h"
#include "memory_alloc.h"

#include <string.h>

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

    Text text;
    if (text_ctor(&text, input_file_name)) {
        ERROR_OCCURRED_CALLING(text_ctor, "returned non-zero value");
        text_dtor(&text);
        printf("Failed to assemble input file, check dump.log for more information\n");
        return EXIT_FAILURE;
    }

    Assembler assembler;
    if (assembler_ctor(&assembler, &text)) {
        ERROR_OCCURRED_CALLING(assembler_ctor, "returned non-zero value");
        text_dtor(&text);
        printf("Failed to assemble input file, check dump.log for more information\n");
        return EXIT_FAILURE;
    }

    char *output_file_name = (char *) calloc(strlen(input_file_name) + strlen("meow") - strlen("asm") + 1, sizeof(char));

    if (output_file_name == NULL) {
        ERROR_OCCURRED_CALLING(calloc, "returned NULL");
        text_dtor(&text);
        assembler_dtor(&assembler);
        printf("Failed to assemble input file, check dump.log for more information\n");
        return EXIT_FAILURE;
    }

    strcpy(output_file_name, input_file_name);
    strcpy(strchr(output_file_name, '.'), ".meow");

    if (assembler_translate_text(&assembler)) {
        ERROR_OCCURRED_CALLING(assembler_translate_text, "returned non-zero value");
        text_dtor(&text);
        assembler_dtor(&assembler);
        printf("Failed to assemble input file, check dump.log for more information\n");
        return EXIT_FAILURE;
    }

    if (assembler_write_assembly(&assembler, output_file_name)) {
        ERROR_OCCURRED_CALLING(assembler_write_assembly, "returned non-zero value");
        text_dtor(&text);
        assembler_dtor(&assembler);
        printf("Failed to assemble input file, check dump.log for more information\n");
        return EXIT_FAILURE;
    }

    printf("Successfully assembled %s into %s\n", input_file_name, output_file_name);
    FREE(output_file_name);
    text_dtor(&text);
    assembler_dtor(&assembler);

    return EXIT_SUCCESS;
}