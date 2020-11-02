#include "Assembler.hpp"

#include <cstring>
#include <cstdlib>

int main(int argc, const char *argv[])
{
    --argc;

    if (argc == 0 || argc > 1) {
        printf("usage: assembler <source_code_file_name>\n");
        return EXIT_SUCCESS;
    }

    const char *inputFileName = argv[1];

    Text text(inputFileName);

    auto outputFileName = new char[strlen(inputFileName) + strlen("meow") - strlen("asm") + 1];
    strcpy(outputFileName, inputFileName);
    strcpy(strchr(outputFileName, '.') + 1, "meow");

    Assembler assembler(&text, inputFileName, outputFileName);
    assembler();

    printf("Successfully assembled %s into %s.\n", inputFileName, outputFileName);
    delete[] outputFileName;

    return EXIT_SUCCESS;
}
