#include "Processor.hpp"

#include "File.hpp"
#include "ReadWrite.hpp"

#include <cassert>
#include <cstdio>

Processor::Processor(const char *inputFileName)
: dataStack(10), callStack(), registers{}, ram{}
{
    assert(inputFileName != nullptr);

    FILE *inputFile = std::fopen(inputFileName, "rb");
    assembly = readBinaryFileToBuf(inputFile);
    assemblySize = *(size_t *)(assembly + sizeof(short) + sizeof(char));
    assembly += sizeof(short) + sizeof(char) + sizeof(size_t);
    rip = assembly;
}

void Processor::operator()()
{
    while (rip - assembly < assemblySize) {
        switch (*rip) {
#define DEFINE_COMMAND(name, code, noArg, processorSrc) \
        case code: {                                    \
            rip += sizeof(char);                        \
                                                        \
            processorSrc                                \
                                                        \
            break;                                      \
        }
#include "Commands.hpp"
#undef DEFINE_COMMAND
        }
    }
}

Processor::~Processor()
{
    delete[] (assembly - sizeof(short) - sizeof(char) - sizeof(size_t));
}
