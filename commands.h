/* DEF_CMD(name, number, n_args, code)  */

DEF_CMD(hlt, 0, 0,
{
    return 0;
})

DEF_CMD(dump, 1, 0,
{
    stack_print(processor->stack);
})

DEF_CMD(  in, 2, 0,
{
   val_t val = 0;
   scanf("%lf", &val);

   stack_push(&processor->stack, val);
})

DEF_CMD(out, 3, 0,
{
    bool error = false;
    val_t val = stack_pop(&processor->stack, &error);

    if (error) {
        ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "pop from empty stack");
        return 1;
    }

    printf("%lf\n", val);
})
DEF_CMD(push, 4, 2,
{
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
            ERROR_OCCURRED_IN_FUNC(processor_execute_assembly,
            "invalid mode");
            return 1;
        }
    }
})

DEF_CMD(pop, 5, 1,
{
    bool error = false;
    val_t val = stack_pop(&processor->stack, &error);

    if (error) {
        ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "pop from empty stack");
        return 1;
    }

    char register_number = *rip;
    rip += sizeof(char);

    processor->r[register_number] = val;
})

DEF_CMD(neg, 6, 0,
{
    bool error = false;
    val_t val = stack_pop(&processor->stack, &error);

    if (error) {
        ERROR_OCCURRED_IN_FUNC(processor_execute_assembly, "pop from empty stack");
        return 1;
    }

    stack_push(&processor->stack, -val);
})

DEF_CMD(add, 7, 0,
{
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
})

DEF_CMD(sub, 8, 0,
{
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
})

DEF_CMD(mul, 9, 0,
{
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
})

DEF_CMD(div, 10, 0,
{
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
})

DEF_CMD(pow, 11, 0,
{
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
})