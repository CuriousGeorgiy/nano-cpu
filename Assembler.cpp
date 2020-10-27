#include "Assembler.hpp"

#include "error.h"
#include "ReadWrite.hpp"
#include "memory_alloc.h"

#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>

Assembler::Assembler(const Text *text, const char *inputFileName, const char *outputFileName)
: nLines(text->nLines), labels((Label *) std::calloc(text->nLines, sizeof(Label))), nLabels(0), lines(text->lines),
  assembly((char *) calloc(nLines * (sizeof(char) + sizeof(char) + sizeof(double) + sizeof(ptrdiff_t)), sizeof(char))),
  translator(assembly), outputFileName(outputFileName)
{
    auto listingFileName = (char *) std::calloc(strlen(inputFileName) + strlen("lst") - strlen("asm") + 1, sizeof(char));

    std::strcpy(listingFileName, inputFileName);
    std::strcpy(strchr(listingFileName, '.') + 1, "lst");

    listing = std::fopen(listingFileName, "w");
    std::free(listingFileName);

    fprintf(listing, "%s -> %s\n\n", inputFileName, outputFileName);
}

Assembler::~Assembler()
{
    std::fclose(listing);
    std::free(assembly);

    for (size_t i = 0; i < nLabels; ++i) {
        std::free(labels[i].name);
    }

    std::free(labels);
}

void Assembler::operator()()
{
    for (size_t i = 0; i < nLines; ++i) {
        char *label = nullptr;
        char *cmdName = nullptr;
        char *cmdArg = nullptr;

        Text::Line line = {nullptr, lines[i].len};
        line.str = (char *) std::calloc(lines[i].len, sizeof(char));
        std::strcpy(line.str, lines[i].str);

        tokenizeLine(&line, &label, &cmdName, &cmdArg);

        if (label != nullptr) {
            labels[nLabels].name = (char *) std::calloc(strlen(label), sizeof(char));
            std::strncpy(labels[nLabels].name, label, strlen(label) - 1);
            labels[nLabels].address = translator - assembly;
            ++nLabels;
        }

        if (cmdName == nullptr) {
            continue;
        }

        fprintf(listing, "%03lld ", translator - assembly);

#define DEFINE_COMMAND(name, code, noArg, processorSrc)         \
{                                                               \
        if (strcmp(cmdName, #name) == 0) {                      \
            translateInstruction(cmdName, code, cmdArg, noArg); \
            continue;                                           \
        }                                                       \
}
#include "Commands.hpp"
#undef DEFINE_COMMAND

        std::free(line.str);
    }

    fprintf(listing, "\n");
    translator = assembly;
    for (size_t i = 0; i < nLines; ++i) {
        char *label = nullptr;
        char *cmdName = nullptr;
        char *cmdArg = nullptr;

        Text::Line line = {nullptr, lines[i].len};
        line.str = (char *) std::calloc(lines[i].len, sizeof(char));
        std::strcpy(line.str, lines[i].str);

        tokenizeLine(&line, &label, &cmdName, &cmdArg);

        if (cmdName == nullptr) {
            continue;
        }

        fprintf(listing, "%03lld ", translator - assembly);

#define DEFINE_COMMAND(name, code, noArg, processorSrc)         \
{                                                               \
        if (strcmp(cmdName, #name) == 0) {                      \
            translateInstruction(cmdName, code, cmdArg, noArg); \
            continue;                                           \
        }                                                       \
}
#include "Commands.hpp"
#undef DEFINE_COMMAND

        std::free(line.str);
    }

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
    return (strcmp(cmdName, "jmp") == 0) || (strcmp(cmdName, "ja") == 0) || (strcmp(cmdName, "jae") == 0) ||
           (strcmp(cmdName, "jb") == 0) || (strcmp(cmdName, "jbe") == 0) || (strcmp(cmdName, "je") == 0) ||
           (strcmp(cmdName, "jne") == 0);
}

void Assembler::translateInstruction(const char *cmdName, char cmdCode, const char *cmdArg, bool noArg)
{
    assert(cmdName != nullptr);

    if (noArg) {
        fprintf(listing, "%2d%s", cmdCode, "     ");
        fprintf(listing, "%s\n", cmdName);
        translateCommand(cmdCode);
        return;
    }

    assert(cmdArg != nullptr);

    fprintf(listing, "%d ", cmdCode);
    translateCommand(cmdCode);

    if (isJumpInstruction(cmdName) || (strcmp(cmdName, "call") == 0)) {
        if (cmdArg[0] != ':') {
            ptrdiff_t jmpAddress = std::atoi(cmdArg);
            fprintf(listing, "%lld ", jmpAddress);
            translateAddress(jmpAddress);
            fprintf(listing, "%s %s\n", cmdName, cmdArg);
            return;
        }

        for (size_t i = 0; i < nLabels; ++i) {
            if (strcmp(labels[i].name, (cmdArg + 1)) == 0) {
                fprintf(listing, "%lld ", labels[i].address);
                translateAddress(labels[i].address);
                fprintf(listing, "%s %s\n", cmdName, cmdArg);
                return;
            }
        }

        fprintf(listing, "%lld %s %s\n", -1ll, cmdName, cmdArg);
        translator += sizeof(ptrdiff_t);
        return;
    }

    if (isRegister(cmdArg)) {
        char regCode = registerStringToCode(cmdArg);

        fprintf(listing, "%c ", '0' + Register);
        fprintf(listing, "%c ", '0' + regCode);

        translateReadWriteMode(Register);
        translateRegister(regCode);
    } else {
        fprintf(listing, "%c ", '0' + Constant);
        fprintf(listing, "%s ", cmdArg);

        translateReadWriteMode(Constant);
        translateConstant(cmdArg);
    }

    fprintf(listing, "%s %s\n", cmdName, cmdArg);
}

void Assembler::translateAddress(ptrdiff_t address)
{
    *(ptrdiff_t *) translator = address;
    translator += sizeof(ptrdiff_t);
}

void Assembler::translateCommand(char cmd)
{
    *translator = cmd;
    translator += sizeof(char);
}

void Assembler::translateConstant(const char *arg)
{
    assert(arg != nullptr);

    *(double *) translator = std::atof(arg);
    translator += sizeof(double);
}

void Assembler::translateReadWriteMode(ReadWriteMode readWriteMode)
{
    *translator = (char) readWriteMode;
    translator += sizeof(char);
}

bool Assembler::isRegister(const char *arg)
{
    assert(arg != nullptr);

    return (strlen(arg) == 3) && (arg[0] == 'r') && ((arg[1] >= 'a') && (arg[1] <= 'd') || (arg[1] >= '0') && (arg[1] <= '2')) &&
           (arg[2] == 'x');
}

char Assembler::registerStringToCode(const char *regStr)
{
    assert(regStr != nullptr);

    if (std::isalpha(regStr[1])) {
        return regStr[1] - 'a';
    }

    return regStr[1] - '0' + 4;
}

void Assembler::translateRegister(char reg)
{
    *translator = reg;
    translator += sizeof(char);
}

void Assembler::writeAssembly()
{
    FILE *output = std::fopen(outputFileName, "wb");

    size_t headerSize = sizeof(short) + sizeof(char) + sizeof(size_t);
    auto header = (char *) std::calloc(headerSize, sizeof(char));
    char *headerWriter = header;

    *((short *) headerWriter) = 'G' + 256 * 'L';
    headerWriter += sizeof(short);

    *headerWriter = 1;
    headerWriter += sizeof(char);

    size_t assemblySize = translator - assembly;
    *((size_t *) headerWriter) = assemblySize;

    fwrite(header, sizeof(char), headerSize, output);
    std::free(header);

    fwrite(assembly, sizeof(char), assemblySize, output);

    fclose(output);
}
