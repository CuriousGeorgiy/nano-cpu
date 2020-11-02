#include "Processor.hpp"

#include <cstdlib>

int main(int argc, const char *argv[])
{
    --argc;

    if (argc == 0 || argc > 1) {
        printf("usage: processor <object_file_name>\n");
        return EXIT_SUCCESS;
    }

    const char *inputFileName = argv[1];

    Processor processor(inputFileName);
    processor();

    printf("Successfully executed input file\n");

    return EXIT_SUCCESS;
}
