#include "Disassembler.hpp"

#include <cstdlib>

int main(int argc, const char *argv[])
{
    --argc;

    if (argc == 0 || argc > 1) {
        printf("usage: disassembler <object_file_name>\n");
        return EXIT_SUCCESS;
    }

    const char *inputFileName = argv[1];

    Disassembler disassembler(inputFileName);
    disassembler();

    printf("Successfully disassembled input file.\n");

    return EXIT_SUCCESS;
}
