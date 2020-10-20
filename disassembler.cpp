#include "disassembler.h"

#include "error.h"
#include "stack.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int disassembler_ctor(Disassembler *disassembler, const char *input_file_name, char *assembly, size_t assembly_size)
{
    assert(disassembler != NULL);
    assert(assembly != NULL);

    char *output_file_name = (char *) calloc(strlen(input_file_name) + strlen("_disassembly") + strlen("asm") - strlen("meow") + 1,
                                             sizeof(char));

    if (output_file_name == NULL) {
        ERROR_OCCURRED_CALLING(calloc, "returned NULL");
        return 1;
    }

    strcpy(output_file_name, input_file_name);
    strcpy(strchr(output_file_name, '.'), "_disassembly.asm");

    disassembler->disassembly = fopen(output_file_name, "w");

    if (disassembler->disassembly == NULL) {
        ERROR_OCCURRED_CALLING(fopen, "returned NULL");
        return 1;
    }

    disassembler->assembly = assembly;
    disassembler->assembly_size = assembly_size;

    return 0;
}

int disassemble_assembly(Disassembler *disassembler)
{
    assert(disassembler != NULL);
    assert(disassembler->assembly != NULL);

    char *rip = disassembler->assembly;

    while (rip - disassembler->assembly < disassembler->assembly_size) {
        switch (*rip) {
            case HLT: {
                rip += sizeof(char);

                fprintf(disassembler->disassembly, "hlt\n");

                break;
            }
            case DUMP: {
                rip += sizeof(char);

                fprintf(disassembler->disassembly, "dump\n");

                break;
            }
            case IN: {
                rip += sizeof(char);

                fprintf(disassembler->disassembly, "in\n");

                break;
            }
            case OUT: {
                rip += sizeof(char);

                fprintf(disassembler->disassembly, "out\n");

                break;
            }
            case PUSH: {
                rip += sizeof(char);

                char mode = *rip;

                rip += sizeof(char);

                switch (mode) {
                    case 0: {
                        val_t val = *(val_t *) rip;

                        fprintf(disassembler->disassembly, "push 0 %lg\n", val);

                        rip += sizeof(val_t);

                        break;
                    }
                    case 1: {
                        char register_number = *rip;

                        fprintf(disassembler->disassembly, "push 1 r%c\n", '0' + register_number);

                        rip += sizeof(char);

                        break;
                    }
                    default: {
                        ERROR_OCCURRED_IN_FUNC(disassemble_assembly, "invalid mode");
                        return 1;
                    }
                }

                break;
            }
            case POP: {
                rip += sizeof(char);

                char register_number = *rip;

                fprintf(disassembler->disassembly, "pop r%c\n", '0' + register_number);

                rip += sizeof(char);

                break;
            }
            case NEG: {
                rip += sizeof(char);

                fprintf(disassembler->disassembly, "neg\n");

                break;
            }
            case ADD: {
                rip += sizeof(char);

                fprintf(disassembler->disassembly, "add\n");

                break;
            }
            case SUB: {
                rip += sizeof(char);

                fprintf(disassembler->disassembly, "sub\n");

                break;
            }
            case MUL: {
                rip += sizeof(char);

                fprintf(disassembler->disassembly, "mul\n");

                break;
            }
            case DIV: {
                rip += sizeof(char);

                fprintf(disassembler->disassembly, "div\n");

                break;
            }
            case POW: {
                rip += sizeof(char);

                fprintf(disassembler->disassembly, "pow\n");

                break;
            }
            default: {
                ERROR_OCCURRED_IN_FUNC(disassemble_assembly, "invalid instruction");
                return 1;
            }
        }
    }

    return 0;
}