#include "Disassembler.hpp"

#include "error.h"
#include "ReadWrite.hpp"
#include "Stack.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>

Disassembler::Disassembler(const char *inputFileName, const char *assembly, size_t assemblySize)
: assembly(assembly), rip(assembly), assemblySize(assemblySize)
{
    assert(inputFileName != nullptr);
    assert(assembly != nullptr);

    char *outputFileName = (char *) std::calloc(strlen(inputFileName) + std::strlen("_disassembly") +
                                                std::strlen("asm") - std::strlen("meow") + 1, sizeof(char));

    std::strcpy(outputFileName, inputFileName);
    std::strcpy(strchr(outputFileName, '.'), "_disassembly.asm");

    disassembly = std::fopen(outputFileName, "w");
    std::free(outputFileName);
}

void Disassembler::operator()()
{
    while (rip - assembly < assemblySize) {
        switch (*rip) {
#define DEFINE_COMMAND(name, code, noArg, processorSrc) \
            case code: {                                    \
                rip += sizeof(char);                        \
                                                            \
                disassembleInstruction(#name, noArg);       \
                                                            \
                break;                                      \
            }
#include "Commands.hpp"
#undef DEFINE_COMMAND
        }
    }
}

void Disassembler::disassembleInstruction(const char *name, bool noArg)
{
    assert(name != nullptr);

    if (noArg) {
        fprintf(disassembly, "%s\n", name);
        return;
    }

    fprintf(disassembly, "%s ", name);

    if (isJumpInstruction(name) || (strcmp(name, "call") == 0)) {
        auto address = *(ptrdiff_t *) rip;
        rip += sizeof(ptrdiff_t);
        fprintf(disassembly, "%lld\n", address);
        return;
    }

    auto readWriteMode = (ReadWriteMode) *rip;
    rip += sizeof(char);

    switch (readWriteMode) {
        case Constant: {
            auto val = *(constant_t *) rip;
            rip += sizeof(constant_t);
            fprintf(disassembly, "%lg\n", val);
            break;
        }
        case Register: {
            char regCode = *rip;
            rip += sizeof(char);
            fprintf(disassembly, "r%cx\n", registerCodeToLetter(regCode));
            break;
        }
    }
}

bool Disassembler::isJumpInstruction(const char *cmdName)
{
    return (strcmp(cmdName, "jmp") == 0) || (strcmp(cmdName, "ja") == 0) || (strcmp(cmdName, "jae") == 0) ||
           (strcmp(cmdName, "jb") == 0) || (strcmp(cmdName, "jbe") == 0) || (strcmp(cmdName, "je") == 0) ||
           (strcmp(cmdName, "jne") == 0);
}

char Disassembler::registerCodeToLetter(char regCode)
{
    return regCode + 'a';
}

Disassembler::~Disassembler()
{
    std::fclose(disassembly);
}
