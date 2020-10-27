#define DATA_STACK_PRINT() dataStack.print()

#define DATA_STACK_PUSH(constant) dataStack.push(constant)

#define CALL_STACK_PUSH(address) callStack.push(address)

#define DATA_STACK_POP() \
[this]() -> constant_t     \
{            \
    bool error = false; \
    auto constant = this->dataStack.pop(&error); \
                  \
    if (error) {    \
    return NAN;                       \
    }                  \
    \
    return constant;\
}()

#define CALL_STACK_POP() \
[this]() -> ptrdiff_t     \
{            \
    bool error = false; \
    ptrdiff_t address = this->callStack.pop(&error); \
                  \
    if (error) {    \
    return -1;                       \
    }                  \
    \
    return address;\
}()

#define ZERO_DIVISION_ERROR() TERMINATE()

#define WRITE_TO_REGISTER(constant) \
do {                        \
    char regCode = *rip;          \
    rip += sizeof(char);            \
    \
    registers[regCode] = (constant);      \
} while (false)

#define TERMINATE() return

#define SCAN(constant) scanf("%lg", &(constant))

#define PRINT(constant) printf("%lg\n", (constant))

#define READ_READ_WRITE_MODE() \
[this]()              \
{                     \
    auto mode = (ReadWriteMode) *this->rip;\
    this->rip += sizeof(char);                                                           \
    return mode;                                                                                \
}()

#define READ_CONSTANT() \
[this]()   \
{            \
    auto constant = *(constant_t *) this->rip;\
    this->rip += sizeof(constant_t);      \
    return constant;            \
}()

#define READ_REGISTER() \
[this]()  \
{            \
    char reg_code = *this->rip;\
    this->rip += sizeof(char); \
    return this->registers[reg_code];            \
}()

#define READ_ADDRESS() \
[this]()  \
{            \
    auto address = *(ptrdiff_t *) this->rip;\
    this->rip += sizeof(ptrdiff_t); \
    return address;            \
}()

#define SET_ADDRESS(address) (rip = assembly + (address))

#define CURRENT_ADDRESS (rip - assembly)

/* DEFINE_COMMAND(name, code, noArg, processorSrc) */

DEFINE_COMMAND(hlt, 0, true,
{
    TERMINATE();
})

DEFINE_COMMAND(dump, 1, true,
{
    DATA_STACK_PRINT();
})

DEFINE_COMMAND(in, 2, true,
{
    constant_t constant = 0;
    SCAN(constant);
    DATA_STACK_PUSH(constant);
})

DEFINE_COMMAND(out, 3, true,
{
    auto constant = DATA_STACK_POP();
    PRINT(constant);
})

DEFINE_COMMAND(push, 4, false,
{
    ReadWriteMode readWriteMode = READ_READ_WRITE_MODE();

    switch (readWriteMode) {
        case Constant: {
            DATA_STACK_PUSH(READ_CONSTANT());
            break;
        }
        case Register: {
            DATA_STACK_PUSH(READ_REGISTER());
            break;
        }
    }
})

DEFINE_COMMAND(pop, 5, false,
{
    ReadWriteMode readWriteMode = READ_READ_WRITE_MODE();
    auto constant = DATA_STACK_POP();
    WRITE_TO_REGISTER(constant);
})

DEFINE_COMMAND(neg, 6, true,
{
    auto constant = DATA_STACK_POP();
    DATA_STACK_PUSH(-constant);
})

DEFINE_COMMAND(add, 7, true,
{
    auto term1 = DATA_STACK_POP();
    auto term2 = DATA_STACK_POP();
    DATA_STACK_PUSH(term1 + term2);
})

DEFINE_COMMAND(sub, 8, true,
{
    auto subtrahend = DATA_STACK_POP();
    auto minuend = DATA_STACK_POP();
    DATA_STACK_PUSH(minuend - subtrahend);
})

DEFINE_COMMAND(mul, 9, true,
{
    auto factor1 = DATA_STACK_POP();
    auto factor2 = DATA_STACK_POP();
    DATA_STACK_PUSH(factor1 * factor2);
})

DEFINE_COMMAND(div, 10, true,
{
    auto divisor = DATA_STACK_POP();
    auto dividend = DATA_STACK_POP();

    if (divisor == 0) {
        ZERO_DIVISION_ERROR();
    }

    DATA_STACK_PUSH(dividend / divisor);
})

DEFINE_COMMAND(pow, 11, true,
{
    auto power = DATA_STACK_POP();
    auto base = DATA_STACK_POP();

    DATA_STACK_PUSH(pow(base, power));
})

DEFINE_COMMAND(jmp, 12, false,
{
    SET_ADDRESS(READ_ADDRESS());
})

DEFINE_COMMAND(ja, 13, false,
{
    auto constant1 = DATA_STACK_POP();
    auto constant2 = DATA_STACK_POP();
    auto address = READ_ADDRESS();

    if (constant1 < constant2) {
        SET_ADDRESS(address);
    }
})

DEFINE_COMMAND(jae, 14, false,
{
    auto constant1 = DATA_STACK_POP();
    auto constant2 = DATA_STACK_POP();
    auto address = READ_ADDRESS();

    if (constant1 <= constant2) {
        SET_ADDRESS(address);
    }
})

DEFINE_COMMAND(jb, 15, false,
{
    auto constant1 = DATA_STACK_POP();
    auto constant2 = DATA_STACK_POP();
    auto address = READ_ADDRESS();

    if (constant1 > constant2) {
        SET_ADDRESS(address);
    }
})

DEFINE_COMMAND(jbe, 16, false,
{
    auto constant1 = DATA_STACK_POP();
    auto constant2 = DATA_STACK_POP();
    auto address = READ_ADDRESS();

    if (constant1 >= constant2) {
        SET_ADDRESS(address);
    }
})

DEFINE_COMMAND(je, 17, false,
{
    auto constant1 = DATA_STACK_POP();
    auto constant2 = DATA_STACK_POP();
    auto address = READ_ADDRESS();

    if (constant1 == constant2) {
        SET_ADDRESS(address);
    }
})

DEFINE_COMMAND(jne, 18, false,
{
    auto constant1 = DATA_STACK_POP();
    auto constant2 = DATA_STACK_POP();
    auto address = READ_ADDRESS();

    if (constant1 != constant2) {
        SET_ADDRESS(address);
    }
})

DEFINE_COMMAND(call, 19, false,
{
    auto callAddress = READ_ADDRESS();
    CALL_STACK_PUSH(CURRENT_ADDRESS);
    SET_ADDRESS(callAddress);
})

DEFINE_COMMAND(ret, 20, true,
{
    auto returnAddress = CALL_STACK_POP();
    SET_ADDRESS(returnAddress);
})
