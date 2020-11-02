#include "File.hpp"

#include <cassert>

long countFileSize(std::FILE *file)
{
    assert(file != nullptr);

    fseek(file, 0, SEEK_END);

    long fileSize = ftell(file);
    rewind(file);

    return fileSize;
}

char *readBinaryFileToBuf(FILE *binaryFile)
{
    assert(binaryFile != nullptr);

    long fileSize = countFileSize(binaryFile);
    auto buffer = new char[fileSize];
    fread(buffer, sizeof(*buffer), fileSize, binaryFile);

    return buffer;
}
