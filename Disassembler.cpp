#include "Disassembler.hpp"

#include "File.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>

Disassembler::Disassembler(const char *inputFileName)
{
    assert(inputFileName != nullptr);

    FILE *inputFile = std::fopen(inputFileName, "rb");
    assembly = readBinaryFileToBuf(inputFile);
    assemblySize = *(size_t *)(assembly + sizeof(short) + sizeof(char));
    assembly += sizeof(short) + sizeof(char) + sizeof(size_t);
    rip = assembly;

    auto outputFileName = new char[strlen(inputFileName) + std::strlen("_disassembly") +
                                   std::strlen("asm") - std::strlen("meow") + 1];
    std::strcpy(outputFileName, inputFileName);
    std::strcpy(strchr(outputFileName, '.'), "_disassembly.asm");
    disassemblyFile = std::fopen(outputFileName, "w");

    delete[] outputFileName;
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

Disassembler::~Disassembler()
{
    std::fclose(disassemblyFile);
    delete[] (assembly - sizeof(short) - sizeof(char) - sizeof(size_t));
}

void Disassembler::disassembleInstruction(const char *name, bool noArg)
{
    assert(name != nullptr);

    std::fprintf(disassemblyFile, "%s ", name);

    if (noArg) {
        disassembleNoArgInstruction();
        return;
    }

    if (isJumpInstruction(name)) {
        disassembleJumpInstructionArg();
        return;
    }

    disassembleReadWriteInstructionArgs();
}

void Disassembler::disassembleJumpInstructionArg()
{
    auto address = *(ptrdiff_t *) rip;
    rip += sizeof(ptrdiff_t);
    std::fprintf(disassemblyFile, "%lld\n", address);
}

bool Disassembler::isJumpInstruction(const char *cmdName)
{
    assert(cmdName != nullptr);

    return (std::strcmp(cmdName, "jmp") == 0) || (std::strcmp(cmdName, "ja") == 0) || (std::strcmp(cmdName, "jae") == 0) ||
           (std::strcmp(cmdName, "jb") == 0) || (std::strcmp(cmdName, "jbe") == 0) || (std::strcmp(cmdName, "je") == 0) ||
           (std::strcmp(cmdName, "jne") == 0) || (std::strcmp(cmdName, "call") == 0);
}

char Disassembler::regCodeToChar(char regCode)
{
    return (regCode >= 0 && regCode <= 3) ? (regCode + 'a') : (regCode - 4 + '0');
}

void Disassembler::disassembleNoArgInstruction()
{
    std::fprintf(disassemblyFile, "\n");
}

void Disassembler::disassembleReadWriteInstructionArgs()
{
    ReadWriteMode readWriteMode = {};
    readWriteMode.mode = *rip;
    rip += sizeof(char);

    if (readWriteMode.ram) {
        std::fprintf(disassemblyFile, "[");
        disassembleReadWriteInstructionValueArg(readWriteMode);
        std::fprintf(disassemblyFile, "]\n");
        return;
    }

    disassembleReadWriteInstructionValueArg(readWriteMode);
    std::fprintf(disassemblyFile, "\n");
}

void Disassembler::disassembleReadWriteInstructionValueArg(ReadWriteMode readWriteMode)
{
    if (readWriteMode.reg & readWriteMode.constant) {
        auto regCode = *rip;
        rip += sizeof(char);
        auto offset = *(constant_t *) rip;
        rip += sizeof(constant_t);
        std::fprintf(disassemblyFile, "r%cx+%lg", regCodeToChar(regCode), offset);
    } else if (readWriteMode.reg & !readWriteMode.constant) {
        auto regCode = *rip;
        rip += sizeof(char);
        std::fprintf(disassemblyFile, "r%cx", regCodeToChar(regCode));
    } else if (readWriteMode.constant & !readWriteMode.reg) {
        auto offset = *(constant_t *) rip;
        rip += sizeof(constant_t);
        std::fprintf(disassemblyFile, "%lg", offset);
    }
}
