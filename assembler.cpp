#include "assembler.h"

#include "error.h"
#include "memory_alloc.h"

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef double val_t;

int assembler_ctor(Assembler *assembler, Text *text, const char *input_file_name, const char *output_file_name)
{
    assert(assembler != NULL);

    char *listing_file_name = (char *) calloc(strlen(input_file_name) + strlen("lst") - strlen("asm") + 1, sizeof(char));

    if (output_file_name == NULL) {
        ERROR_OCCURRED_CALLING(calloc, "returned NULL");
        return 1;
    }

    strcpy(listing_file_name, input_file_name);
    strcpy(strchr(listing_file_name, '.') + 1, "lst");

    assembler->listing = fopen(listing_file_name, "w");
    FREE(listing_file_name);

    if (assembler->listing == NULL) {
        ERROR_OCCURRED_CALLING(fopen, "returned NULL");
        return 1;
    }

    fprintf(assembler->listing, "%s -> %s\n\n", input_file_name, output_file_name);

    assembler->assembly = (char *) calloc(text->n_lines * (sizeof(char) + sizeof(char) + sizeof(double)), sizeof(char));

    if (assembler->assembly == NULL) {
        ERROR_OCCURRED_CALLING(calloc, "returned NULL");
        return 1;
    }

    assembler->translator = assembler->assembly;
    assembler->lines = text->lines;
    assembler->n_lines = text->n_lines;

    return 0;
}

void assembler_dtor(Assembler *assembler)
{
    assert(assembler != NULL);

    FREE(assembler->assembly);
    assembler->translator = NULL;
    assembler->lines = NULL;

    if (fclose(assembler->listing)) {
        ERROR_OCCURRED_CALLING(fclose, "returned non-zero value");
    }
}

int tokenize_line(Line *line, char **cmd, char **args)
{
    assert(line != NULL);
    assert(cmd != NULL);
    assert(args != NULL);

    char *instruction = strtok(line->str, ";");

    if (instruction == NULL) {
        ERROR_OCCURRED_CALLING(strtok, "returned NULL unexpectedly");
        return 1;
    }

    *cmd = strtok(instruction, " ");
    *args =  strtok(NULL, "");

    return 0;
}

bool is_instruction_mode(char *mode) {
    assert(mode != NULL);

    return (strlen(mode) == 1) && (*mode >= '0') && (*mode <= '1');
}

int parse_args(char *args, unsigned n_args, push_mode *mode, char **val)
{
    assert(args != NULL);
    assert(val != NULL);
    assert(n_args > 0);

    switch (n_args) {
        case 1: {
            *val = args;
            return 0;
        }
        case 2: {
            assert(mode != NULL);

            char *str_mode = NULL;

            str_mode = strtok(args, " ");

            if (str_mode == NULL) {
                ERROR_OCCURRED_CALLING(strtok, "returned NULL unexpectedly");
                return 1;
            }

            if (!is_instruction_mode(str_mode)) {
                ERROR_OCCURRED_IN_FUNC(assembler_parse_arguments, "invalid instruction mode");
                return 1;
            }

            *mode = (push_mode) (*str_mode - '0');

            *val = strtok(NULL, " ");

            if (*val == NULL) {
                ERROR_OCCURRED_CALLING(strtok, "returned NULL unexpectedly");
                return 1;
            }

            return 0;
        }
        default: {
            ERROR_OCCURRED_IN_FUNC(assembler_parse_aguments, "invalid number of arguments");
            return 1;
        }
    }
}

void assembler_translate_command(Assembler *assembler, command cmd)
{
    assert(assembler != NULL);

    *assembler->translator = (char) cmd;

    assembler->translator += sizeof(char);
}

int assembler_translate_value(Assembler *assembler, const char *str_val)
{
    assert(assembler != NULL);
    assert(str_val != NULL);

    val_t val = strtod(str_val, NULL);

    if (!isfinite(val)) {
        ERROR_OCCURRED_IN_FUNC(assembler_translate_argument, "invalid instruction argument");
        return 1;
    }

    *((val_t *) assembler->translator) = val;
    fprintf(assembler->listing, "%s", str_val);

    assembler->translator += sizeof(val_t);

    return 0;
}

void assembler_translate_mode(Assembler *assembler, push_mode mode)
{
    assert(assembler != NULL);

    *assembler->translator = mode;
    fprintf(assembler->listing, "%c ", '0' + mode);

    assembler->translator += sizeof(char);
}

bool is_register(const char *val)
{
    assert(val != NULL);

    return strlen(val) == 2 && (*val == 'r') && (val[1] >= '0') && (val[1] < '4');
}

char register_number(const char *val)
{
    assert(val != NULL);

    return val[1] - '0';
}

void assembler_translate_register(Assembler *assembler, char register_number)
{
    assert(assembler != NULL);

    *assembler->translator = register_number;
    fprintf(assembler->listing, "%c%c", 'r', '0' + register_number);

    assembler->translator += sizeof(char);
}

int assembler_translate_text(Assembler *assembler)
{
    assert(assembler != NULL);

    for (size_t i = 0; i < assembler->n_lines; ++i) {
        char *cmd = NULL;
        char *args = NULL;

        if (tokenize_line(assembler->lines + i, &cmd, &args)) {
            ERROR_OCCURRED_CALLING(assembler_tokenize_line, "returned non-zero value");
            return 1;
        }

        if (cmd == NULL) {
            continue;
        }

        fprintf(assembler->listing, "%05d ", assembler->translator - assembler->assembly);

        if (strcmp(cmd, "hlt") == 0) {
            fprintf(assembler->listing, "%d ", HLT);
            assembler_translate_command(assembler, HLT);
            fprintf(assembler->listing, "%s", cmd);
        } else if (strcmp(cmd, "dump") == 0) {
            fprintf(assembler->listing, "%d ", DUMP);
            assembler_translate_command(assembler, DUMP);
            fprintf(assembler->listing, "%s", cmd);
        } else if (strcmp(cmd, "in") == 0) {
            fprintf(assembler->listing, "%d ", IN);
            assembler_translate_command(assembler, IN);
            fprintf(assembler->listing, "%s", cmd);
        } else if (strcmp(cmd, "out") == 0) {
            fprintf(assembler->listing, "%d ", OUT);
            assembler_translate_command(assembler, OUT);
            fprintf(assembler->listing, "%s", cmd);
        } else if (strcmp(cmd, "push") == 0) {
            fprintf(assembler->listing, "%d ", PUSH);

            if (args == NULL) {
                ERROR_OCCURRED_IN_FUNC(assembler_translate_text, "push instruction without mode and value");
                return 1;
            }

            push_mode mode = VAL;
            char *val = NULL;

            if (parse_args(args, 2, &mode, &val)) {
                ERROR_OCCURRED_CALLING(assembler_parse_arguments, "returned nonzero value");
                return 1;
            }

            fprintf(assembler->listing, "%c ", '0' + mode);

            if (is_register(val)) {
                fprintf(assembler->listing, "%c ", val[1]);
            } else {
                fprintf(assembler->listing, "%s ", val);
            }

            assembler_translate_command(assembler, PUSH);
            fprintf(assembler->listing, "%s ", cmd);

            assembler_translate_mode(assembler, mode);

            switch (mode) {
                case VAL: {
                    if (assembler_translate_value(assembler, val)) {
                        ERROR_OCCURRED_CALLING(assembler_translate_value, "returned nonzero value");
                        return 1;
                    }

                    break;
                }
                case REG: {
                    if (!is_register(val)) {
                        ERROR_OCCURRED_IN_FUNC(assembler_translate_text, "push with invalid register value");
                        return 1;
                    }

                    assembler_translate_register(assembler, register_number(val));
                    break;
                }
                default: {
                    ERROR_OCCURRED_IN_FUNC(assembler_translate_text, "invalid push mode");
                    return 1;
                }
            }
        }  else if (strcmp(cmd, "pop") == 0) {
            fprintf(assembler->listing, "%d ", POP);

            if (args == NULL) {
                ERROR_OCCURRED_IN_FUNC(translate, "pop without register value");
                return 1;
            }

            char *val = NULL;

            if (parse_args(args, 1, NULL, &val)) {
                ERROR_OCCURRED_CALLING(assembler_parse_arguments, "returned nonzero value");
                return 1;
            }

            if (!is_register(val)) {
                ERROR_OCCURRED_IN_FUNC(translate, "pop with invalid register value");
                return 1;
            }

            fprintf(assembler->listing, "%s ", val);

            assembler_translate_command(assembler, POP);
            fprintf(assembler->listing, "%s ", cmd);

            assembler_translate_register(assembler, register_number(val));
        } else if (strcmp(cmd, "neg") == 0) {
            fprintf(assembler->listing, "%d ", NEG);
            assembler_translate_command(assembler, NEG);
            fprintf(assembler->listing, "%s", cmd);
        }  else if (strcmp(cmd, "add") == 0) {
            fprintf(assembler->listing, "%d ", ADD);
            assembler_translate_command(assembler, ADD);
            fprintf(assembler->listing, "%s", cmd);
        } else if (strcmp(cmd, "sub") == 0) {
            fprintf(assembler->listing, "%d ", SUB);
            assembler_translate_command(assembler, SUB);
            fprintf(assembler->listing, "%s", cmd);
        }  else if (strcmp(cmd, "mul") == 0) {
            fprintf(assembler->listing, "%d ", MUL);
            assembler_translate_command(assembler, MUL);
            fprintf(assembler->listing, "%s", cmd);
        } else if (strcmp(cmd, "div") == 0) {
            fprintf(assembler->listing, "%d ", DIV);
            assembler_translate_command(assembler, DIV);
            fprintf(assembler->listing, "%s", cmd);
        } else if (strcmp(cmd, "pow") == 0) {
            fprintf(assembler->listing, "%d ", POW);
            assembler_translate_command(assembler, POW);
            fprintf(assembler->listing, "%s", cmd);
        } else {
            ERROR_OCCURRED_IN_FUNC(assembler_translate_text, "invalid command");
            return 1;
        }

        fprintf(assembler->listing, "\n");
    }

    return 0;
}

int assembler_write_assembly(Assembler *assembler, char *output_file_name)
{
    assert(assembler != NULL);
    assert(output_file_name != NULL);

    FILE *output = fopen(output_file_name, "wb");

    if (output == NULL) {
        ERROR_OCCURRED_CALLING(fopen, "returned NULL");
    }

    size_t header_size = sizeof(short) + sizeof(char) + sizeof(size_t);
    char *header = (char *) calloc(header_size, sizeof(*header));

    if (header == NULL) {
        ERROR_OCCURRED_CALLING(calloc, "returned NULL");
        return 1;
    }

    char *header_writer = header;

    *((short *) header_writer) = 'G' + 256 * 'L';
    header_writer += sizeof(short);

    *header_writer = 1;
    header_writer += sizeof(char);


    size_t assembly_size = assembler->translator - assembler->assembly;
    *((size_t *) header_writer) = assembly_size;

    fwrite(header, sizeof(*header), header_size, output);
    free(header);

    fwrite(assembler->assembly, sizeof(*assembler->assembly), assembly_size, output);
    fclose(output);

    return 0;
}