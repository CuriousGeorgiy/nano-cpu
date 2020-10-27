#include "Disassembler.hpp"
#include "error.h"
#include "file.h"
#include "log.h"
#include "memory_alloc.h"

#include <cassert>
#include <cstdlib>

char *readAssemblyFileToBuffer(FILE *input);

int main(int argc, const char *argv[])
{
    --argc;

    if (argc == 0 || argc > 1) {
        printf("Specify input file\n");
        return EXIT_SUCCESS;
    }

    const char *inputFileName = argv[1];
    FILE *input = std::fopen(inputFileName, "rb");

    char *assembly = readAssemblyFileToBuffer(input);
    size_t assemblySize = *(size_t *)(assembly + sizeof(short) + sizeof(char));

    Disassembler disassembler(inputFileName, assembly + sizeof(short) + sizeof(char) + sizeof(size_t), assemblySize);
    disassembler();

    printf("Successfully disassembled input file\n");

    std::free(assembly);

    return EXIT_SUCCESS;
}

char *readAssemblyFileToBuffer(FILE *input)
{
    assert(input != nullptr);

    long fileSize = countFileSize(input);
    char *buffer = (char *) std::calloc(fileSize, sizeof(char));
    fread(buffer, sizeof(*buffer), fileSize, input);

    return buffer;
}
