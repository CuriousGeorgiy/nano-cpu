#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <stdio.h>

struct Disassembler {
    char *assembly;
    size_t assembly_size;
    FILE *disassembly;
};

enum command {
    HLT,
    DUMP,
    IN,
    OUT,
    PUSH,
    POP,
    NEG,
    ADD,
    SUB,
    MUL,
    DIV,
    POW
};

int disassembler_ctor(Disassembler *disassembler, const char *input_file_name, char *assembly, size_t assembly_size);
int disassemble_assembly(Disassembler *disassembler);

#endif /* DISASSEMBLER_H */
