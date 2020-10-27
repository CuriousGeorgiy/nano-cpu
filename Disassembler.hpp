#ifndef DISASSEMBLER_HPP
#define DISASSEMBLER_HPP

#include <cstdio>

class Disassembler {
public:
    Disassembler(const char *inputFileName, const char *assembly, size_t assemblySize);

    Disassembler(const Disassembler &) = delete;
    Disassembler(Disassembler &&) = delete;

    Disassembler &operator=(const Disassembler &) = delete;
    Disassembler &operator=(Disassembler &&) = delete;

    ~Disassembler();

    void operator()();
private:
    const char *assembly;
    const char *rip;
    const size_t assemblySize;
    std::FILE *disassembly;

    void disassembleInstruction(const char *name, bool noArg);

    static bool isJumpInstruction(const char *cmdName);
    static char registerCodeToLetter(char regCode);
};

#endif /* DISASSEMBLER_HPP */
