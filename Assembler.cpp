#include "Assembler.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>

Assembler::Assembler(const Text *text, const char *inputFileName, const char *outputFileName)
: nLines(text->nLines), labels(new Label[text->nLines]), nLabels(0), lines(text->lines),
  assembly(new char[nLines * std::max(sizeof(char) + sizeof(char) + sizeof(char) + sizeof(constant_t), sizeof(ptrdiff_t))]),
  translator(assembly), outputFileName(outputFileName)
{
    auto listingFileName = new char[strlen(inputFileName) + strlen("lst") - strlen("asm") + 1];
    std::strcpy(listingFileName, inputFileName);
    std::strcpy(strchr(listingFileName, '.') + 1, "lst");

    listingFile = std::fopen(listingFileName, "w");
    delete[] listingFileName;

    std::fprintf(listingFile, "%s -> %s\n\n", inputFileName, outputFileName);
}
Assembler::~Assembler()
{
    std::fclose(listingFile);
    std::free(assembly);

    for (size_t i = 0; i < nLabels; ++i) {
        std::free(labels[i].name);
    }

    std::free(labels);
}

void Assembler::operator()()
{
    pass(1);

    std::fprintf(listingFile, "\n");
    translator = assembly;

    pass(2);

    writeAssembly();
}

void Assembler::tokenizeLine(const Text::Line *line, char **label, char **cmd, char **arg)
{
    assert(line != nullptr);
    assert(label != nullptr);
    assert(cmd != nullptr);
    assert(arg != nullptr);

    char *instruction = std::strtok(line->str, ";");
    *label = std::strtok(instruction, " ");

    if ((*label)[strlen(*label) - 1] != ':') {
        *cmd = *label;
        *label = nullptr;
        *arg = std::strtok(nullptr, " ");
        return;
    }

    *cmd = std::strtok(nullptr, " ");
    *arg = std::strtok(nullptr, " ");
}

bool Assembler::isJumpInstruction(const char *cmdName)
{
    return (std::strcmp(cmdName, "jmp") == 0) || (std::strcmp(cmdName, "ja") == 0) || (std::strcmp(cmdName, "jae") == 0) ||
           (std::strcmp(cmdName, "jb") == 0) || (std::strcmp(cmdName, "jbe") == 0) || (std::strcmp(cmdName, "je") == 0) ||
           (std::strcmp(cmdName, "jne") == 0) || (std::strcmp(cmdName, "call") == 0);
}

void Assembler::translateInstruction(const char *cmdName, char cmdCode, char *cmdArg, bool noArg)
{
    assert(cmdName != nullptr);

    if (noArg) {
        translateNoArgInstruction(cmdName, cmdCode);
        return;
    }

    assert(cmdArg != nullptr);

    translateArgInstruction(cmdName, cmdCode, cmdArg);
}

void Assembler::translateAddress(ptrdiff_t address)
{
    *(ptrdiff_t *) translator = address;
    translator += sizeof(ptrdiff_t);
}

void Assembler::translateCmd(char cmd)
{
    *translator = cmd;
    translator += sizeof(char);
}

void Assembler::translateConst(const char *arg)
{
    assert(arg != nullptr);

    *(double *) translator = std::atof(arg);
    translator += sizeof(double);
}

void Assembler::translateReadWriteMode(ReadWriteMode readWriteMode)
{
    *translator = readWriteMode.mode;
    translator += sizeof(char);
}

bool Assembler::isReg(const char *arg)
{
    assert(arg != nullptr);

    return (arg[0] == 'r') && ((arg[1] >= 'a') && (arg[1] <= 'd') || (arg[1] >= '0') && (arg[1] <= '2')) && (arg[2] == 'x');
}

char Assembler::regStrToCode(const char *regStr)
{
    assert(regStr != nullptr);

    if (std::isalpha(regStr[1])) {
        return regStr[1] - 'a';
    }

    return regStr[1] - '0' + 4;
}

void Assembler::translateReg(char reg)
{
    *translator = reg;
    translator += sizeof(char);
}

void Assembler::writeAssembly()
{
    FILE *output = std::fopen(outputFileName, "wb");

    size_t headerSize = sizeof(short) + sizeof(char) + sizeof(size_t);
    auto header =  new char[headerSize];
    char *headerWriter = header;

    *((short *) headerWriter) = 'G' + 256 * 'L';
    headerWriter += sizeof(short);

    *headerWriter = 1;
    headerWriter += sizeof(char);

    size_t assemblySize = translator - assembly;
    *((size_t *) headerWriter) = assemblySize;

    fwrite(header, sizeof(*header), headerSize, output);
    delete[] header;

    fwrite(assembly, sizeof(*assembly), assemblySize, output);

    fclose(output);
}

bool Assembler::isRam(const char *arg)
{
    return (arg[0] == '[') && (arg[strlen(arg) - 1] == ']');
}

bool Assembler::isRegOffset(const char *arg)
{
    return std::strchr(arg, '+') != nullptr;
}

void Assembler::translateNoArgInstruction(const char *cmdName, char cmdCode)
{
    assert(cmdName);

    std::fprintf(listingFile, "0x%08X %s\n", cmdCode, cmdName);
    translateCmd(cmdCode);
}

void Assembler::translateArgInstruction(const char *cmdName, char cmdCode, char *cmdArg)
{
    std::fprintf(listingFile, "0x%08X ", cmdCode);
    translateCmd(cmdCode);

    if (isJumpInstruction(cmdName)) {
        translateJumpInstructionArg(cmdName, cmdArg);
        return;
    }

    translateReadWriteInstructionArg(cmdName, cmdArg);
}

void Assembler::translateJumpInstructionArg(const char *cmdName, const char *cmdArg)
{
    if (cmdArg[0] != ':') {
        translateJumpInstructionAddress(cmdName, cmdArg);
        return;
    }

    cmdArg += 1;

    translateJumpInstructionLabel(cmdName, cmdArg);
}

void Assembler::translateReadWriteInstructionArg(const char *cmdName, char *cmdArg)
{
    ReadWriteMode readWriteMode = {};

    if (isRam(cmdArg)) {
        readWriteMode.ram = 1;
        cmdArg += 1;
        cmdArg[strlen(cmdArg) - 1] = '\0';
    }

    if (isReg(cmdArg)) {
        translateReadWriteInstructionReg(cmdArg, readWriteMode);
    } else {
        translateReadWriteInstructionConst(cmdArg, readWriteMode);
    }

    if (readWriteMode.ram) {
        std::fprintf(listingFile, "%s [%s]\n", cmdName, cmdArg);
    } else {
        std::fprintf(listingFile, "%s %s\n", cmdName, cmdArg);
    }
}

void Assembler::translateReadWriteInstructionReg(char *cmdArg, ReadWriteMode readWriteMode)
{
    char regCode = regStrToCode(cmdArg);
    readWriteMode.reg = 1;

    if (isRegOffset(cmdArg)) {
        translateReadWriteInstructionRegWithOffset(cmdArg, readWriteMode, regCode);
        return;
    }

    std::fprintf(listingFile, "0x%08X 0x%08X ", '0' + readWriteMode.mode, '0' + regCode);

    translateReadWriteMode(readWriteMode);
    translateReg(regCode);
}

void Assembler::translateReadWriteInstructionConst(const char *cmdArg, ReadWriteMode readWriteMode)
{
    readWriteMode.constant = 1;
    std::fprintf(listingFile, "0x%08X 0x%08llX ", '0' + readWriteMode.mode, (unsigned long long) std::atof(cmdArg));

    translateReadWriteMode(readWriteMode);
    translateConst(cmdArg);
}

void Assembler::translateReadWriteInstructionRegWithOffset(char *cmdArg, ReadWriteMode readWriteMode, char regCode)
{
    readWriteMode.constant = 1;

    char *plus = std::strchr(cmdArg, '+');
    *plus = '\0';
    char *constant = plus + 1;

    std::fprintf(listingFile, "0x%08X 0x%08X 0x%08llX ", '0' + readWriteMode.mode, '0' + regCode, (unsigned long long) std::atof(constant));

    translateReadWriteMode(readWriteMode);
    translateReg(regCode);
    translateConst(constant);
}

bool Assembler::translateLabel(const char *cmdName, const char *label)
{
    for (size_t i = 0; i < nLabels; ++i) {
        if (std::strcmp(labels[i].name, label) == 0) {
            std::fprintf(listingFile, "0x%08tX %s %s\n", labels[i].address, cmdName, label);
            translateAddress(labels[i].address);
            return true;
        }
    }

    return false;
}

void Assembler::translateDummyLabel(const char *cmdName, const char *label)
{
    std::fprintf(listingFile, "%lld %s %s\n", -1ll, cmdName, label);
    translator += sizeof(ptrdiff_t);
}

void Assembler::translateJumpInstructionAddress(const char *cmdName, const char *cmdArg)
{
    ptrdiff_t jmpAddress = std::atoi(cmdArg);
    std::fprintf(listingFile, "0x%08tX %s %s\n", jmpAddress, cmdName, cmdArg);
    translateAddress(jmpAddress);
}

void Assembler::translateJumpInstructionLabel(const char *cmdName, const char *label)
{
    if (!translateLabel(cmdName, label)) {
        translateDummyLabel(cmdName, label);
    }
}

void Assembler::pass(unsigned passNo)
{
    std::fprintf(listingFile, "Pass %u:\n", passNo);

    for (size_t i = 0; i < nLines; ++i) {
        char *label = nullptr;
        char *cmdName = nullptr;
        char *cmdArg = nullptr;

        Text::Line line = {nullptr, lines[i].len};
        line.str = new char[lines[i].len];
        std::strcpy(line.str, lines[i].str);

        tokenizeLine(&line, &label, &cmdName, &cmdArg);

        if (passNo == 1) {
            if (label != nullptr) {
                addLabel(label);
            }
        }

        if (cmdName == nullptr) {
            continue;
        }

        std::fprintf(listingFile, "%03lld ", translator - assembly);

#define DEFINE_COMMAND(name, code, noArg, processorSrc)             \
        {                                                           \
            if (std::strcmp(cmdName, #name) == 0) {                      \
                translateInstruction(cmdName, code, cmdArg, noArg); \
                continue;                                           \
            }                                                       \
        }
#include "Commands.hpp"
#undef DEFINE_COMMAND

        delete[] line.str;
    }
}

void Assembler::addLabel(const char *label)
{
    labels[nLabels].name = (char *) std::calloc(strlen(label), sizeof(char));
    std::strncpy(labels[nLabels].name, label, strlen(label) - 1);
    labels[nLabels].address = translator - assembly;
    ++nLabels;
}
