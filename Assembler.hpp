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
    std::FILE *listing;
    const char *outputFileName;

    void translateAddress(ptrdiff_t address);
    void translateCommand(char cmd);
    void translateConstant(const char *arg);
    void translateReadWriteMode(ReadWriteMode readWriteMode);
    void translateRegister(char reg);
    void translateInstruction(const char *cmdName, char cmdCode, const char *cmdArg, bool noArg);

    void writeAssembly();

    static void tokenizeLine(const Text::Line *line, char **label, char **cmd, char **arg);
    static bool isRegister(const char *arg);
    static bool isJumpInstruction(const char *cmdName);
    static char registerStringToCode(const char *regStr);
};

#endif /* ASSEMBLER_HPP */
