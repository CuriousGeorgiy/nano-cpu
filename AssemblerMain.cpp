#include "Assembler.hpp"
#include "error.h"

#include <cstring>
#include <cstdlib>

int main(int argc, const char *argv[])
{
    --argc;

    if (argc == 0 || argc > 1) {
        printf("Specify input file\n");
        return EXIT_SUCCESS;
    }

    const char *inputFileName = argv[1];

    Text text(inputFileName);

    auto outputFileName = (char *) calloc(strlen(inputFileName) + strlen("meow") - strlen("asm") + 1, sizeof(char));

    strcpy(outputFileName, inputFileName);
    strcpy(strchr(outputFileName, '.') + 1, "meow");

    Assembler assembler(&text, inputFileName, outputFileName);
    assembler();

    printf("Successfully assembled %s into %s\n", inputFileName, outputFileName);
    std::free(outputFileName);

    return EXIT_SUCCESS;
}
