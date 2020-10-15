#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "stack.h"

struct Processor {
    Stack *stack;
    char *assembly;
    size_t assembly_size;

    double r[4];
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

int processor_ctor(Processor *processor, char *assembly, size_t assembly_size);
void processor_dtor(Processor *processor);
int processor_execute_assembly(Processor *processor);

#endif /* PROCESSOR_H */
