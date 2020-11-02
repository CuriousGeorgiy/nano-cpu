#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include "Text.hpp"
#include "ReadWrite.hpp"

#include <cstdio>

class Assembler {
public:
    Assembler(const Text *text, const char *inputFileName, const char *outputFileName);

    Assembler(const Assembler &) = delete;
    Assembler(Assembler &&) = delete;

    Assembler &operator=(const Assembler &) = delete;
    Assembler &operator=(Assembler &&) = delete;

    ~Assembler();

    void operator()();
private:
    struct Label {
        char *name;
        ptrdiff_t address;
    };

    Label *labels;
    size_t nLabels;
    const Text::Line *lines;
    const size_t nLines;
    char *assembly;
    char *translator;
    std::FILE *listingFile;
    const char *outputFileName;

    void pass(unsigned passNo);
    void addLabel(const char *label);

    void translateAddress(ptrdiff_t address);
    void translateCmd(char cmd);
    void translateConst(const char *arg);
    void translateReadWriteMode(ReadWriteMode readWriteMode);
    void translateReg(char reg);
    void translateInstruction(const char *cmdName, char cmdCode, char *cmdArg, bool noArg);
    void translateNoArgInstruction(const char *cmdName, char cmdCode);
    void translateArgInstruction(const char *cmdName, char cmdCode, char *cmdArg);
    void translateJumpInstructionArg(const char *cmdName, const char *cmdArg);
    void translateJumpInstructionAddress(const char *cmdName, const char *cmdArg);
    void translateJumpInstructionLabel(const char *cmdName, const char *label);
    void translateReadWriteInstructionArg(const char *cmdName, char *cmdArg);
    void translateReadWriteInstructionReg(char *cmdArg, ReadWriteMode readWriteMode);
    void translateReadWriteInstructionRegWithOffset(char *cmdArg, ReadWriteMode readWriteMode, char regCode);
    void translateReadWriteInstructionConst(const char *cmdArg, ReadWriteMode readWriteMode);
    bool translateLabel(const char *cmdName, const char *label);
    void translateDummyLabel(const char *cmdName, const char *label);

    void writeAssembly();

    static void tokenizeLine(const Text::Line *line, char **label, char **cmd, char **arg);
    static bool isRam(const char *arg);
    static bool isRegOffset(const char *arg);
    static bool isReg(const char *arg);
    static bool isJumpInstruction(const char *cmdName);
    static char regStrToCode(const char *regStr);
};

#endif /* ASSEMBLER_HPP */
