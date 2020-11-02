#ifndef DISASSEMBLER_HPP
#define DISASSEMBLER_HPP

#include <cstdio>
#include "ReadWrite.hpp"

class Disassembler {
public:
    Disassembler(const char *inputFileName);

    Disassembler(const Disassembler &) = delete;
    Disassembler(Disassembler &&) = delete;

    Disassembler &operator=(const Disassembler &) = delete;
    Disassembler &operator=(Disassembler &&) = delete;

    ~Disassembler();

    void operator()();
private:
    char *assembly;
    const char *rip;
    size_t assemblySize;
    std::FILE *disassemblyFile;

    void disassembleInstruction(const char *name, bool noArg);
    void disassembleNoArgInstruction();
    void disassembleJumpInstructionArg();
    void disassembleReadWriteInstructionArgs();
    void disassembleReadWriteInstructionValueArg(ReadWriteMode readWriteMode);

    static bool isJumpInstruction(const char *cmdName);
    static char regCodeToChar(char regCode);
};

#endif /* DISASSEMBLER_HPP */
