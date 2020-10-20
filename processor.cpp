#include "processor.h"

#include "error.h"

#include <assert.h>


int processor_ctor(Processor *processor, char *assembly, size_t assembly_size)
{
    assert(processor != NULL);
    assert(assembly != NULL);

    processor->stack = new_stack();

    if (processor->stack == NULL) {
        ERROR_OCCURRED_CALLING(new_stack, "returned NULL");
        return 1;
    }

    processor->assembly = assembly;
    processor->assembly_size = assembly_size;

    return 0;
}

void processor_dtor(Processor *processor)
{
    assert(processor != NULL);

    delete_stack(&processor->stack);
}

int processor_execute_assembly(Processor *processor)
{
    assert(processor != NULL);
    assert(processor->assembly != NULL);

    char *rip = processor->assembly - sizeof(char);

    while (rip - processor->assembly < processor->assembly_size) {
        rip += sizeof(char);
        switch (*rip) {
            case HLT: {
                return 0;
            }
            case DUMP: {
                stack_print(processor->stack);

                break;
            }
            case IN: {
                val_t val = 0;
                scanf("%lf", &val);

                stack_push(&processor->stack, val);

                break;
            }
            case OUT: {
                bool error = false;
                val_t val = stack_pop(&processor->stack, &error);

                if (error) {
                    ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "pop from empty stack");
                    return 1;
                }

                printf("%lf\n", val);

                break;
            }
            case PUSH: {
                push_mode mode = (push_mode) *rip;

                rip += sizeof(char);

                switch (mode) {
                    case VAL: {
                        val_t val = *(val_t *) rip;
                        stack_push(&processor->stack, val);

                        rip += sizeof(val_t);

                        break;
                    }
                    case REG: {
                        char register_number = *rip;
                        stack_push(&processor->stack, processor->r[register_number]);

                        rip += sizeof(char);

                        break;
                    }
                    default: {
                        ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "invalid mode");
                        return 1;
                    }
                }

                break;
            }
            case POP: {
                bool error = false;
                val_t val = stack_pop(&processor->stack, &error);

                if (error) {
                    ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "pop from empty stack");
                    return 1;
                }

                char register_number = *rip;
                rip += sizeof(char);

                processor->r[register_number] = val;

                break;
            }
            case NEG: {
                bool error = false;
                val_t val = stack_pop(&processor->stack, &error);

                if (error) {
                    ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "pop from empty stack");
                    return 1;
                }

                stack_push(&processor->stack, -val);

                break;
            }
            case ADD: {
                bool error = false;

                val_t val1 = stack_pop(&processor->stack, &error);

                if (error) {
                    ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "pop from empty stack");
                    return 1;
                }

                val_t val2 = stack_pop(&processor->stack, &error);

                if (error) {
                    ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "pop from empty stack");
                    return 1;
                }

                stack_push(&processor->stack, val1 + val2);

                break;
            }
            case SUB: {
                bool error = false;
                val_t subtrahend = stack_pop(&processor->stack, &error);

                if (error) {
                    ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "pop from empty stack");
                    return 1;
                }

                val_t minuend = stack_pop(&processor->stack, &error);

                if (error) {
                    ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "pop from empty stack");
                    return 1;
                }

                stack_push(&processor->stack, minuend - subtrahend);

                break;
            }
            case MUL: {
                bool error = false;
                val_t val1 = stack_pop(&processor->stack, &error);

                if (error) {
                    ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "pop from empty stack");
                    return 1;
                }

                val_t val2 = stack_pop(&processor->stack, &error);

                if (error) {
                    ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "pop from empty stack");
                    return 1;
                }

                stack_push(&processor->stack, val1 * val2);

                break;
            }
            case DIV: {
                bool error = false;
                val_t divisor = stack_pop(&processor->stack, &error);

                if (error) {
                    ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "pop from empty stack");
                    return 1;
                }

                if (divisor == 0) {
                    ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "zero division");
                    return 1;
                }

                val_t dividend = stack_pop(&processor->stack, &error);

                if (error) {
                    ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "pop from empty stack");
                    return 1;
                }

                stack_push(&processor->stack, dividend / divisor);

                break;
            }
            case POW: {
                bool error = false;
                val_t power = stack_pop(&processor->stack, &error);

                if (error) {
                    ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "pop from empty stack");
                    return 1;
                }

                val_t base = stack_pop(&processor->stack, &error);

                if (error) {
                    ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "pop from empty stack");
                    return 1;
                }

                stack_push(&processor->stack, pow(base, power));

                break;
            }
            default: {
                ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "invalid instruction");
                return 1;
            }
        }
    }

    return 0;
}