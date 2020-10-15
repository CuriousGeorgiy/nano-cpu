#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "text.h"

#include <stdio.h>

struct Assembler {
    Line *lines;
    size_t n_lines;
    char *assembly;
    char *translator;
    FILE *listing;
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

int assembler_ctor(Assembler *assembler, Text *text);
void assembler_dtor(Assembler *assembler);
int assembler_translate_text(Assembler *assembler);
int assembler_write_assembly(Assembler *assembler, char *output_file_name);

#endif /* ASSEMBLER_H */
