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

#define WRITE_VALUE_TO_REGISTER(constant) \
do {                        \
    char regCode = *rip;          \
    rip += sizeof(char);            \
    \
    registers[regCode] = (constant);      \
} while (false)

#define TERMINATE() return

#define SCAN(constant) std::scanf("%lg", &(constant))

#define PRINT(constant) std::printf("%lg\n", (constant))

#define READ_READ_WRITE_MODE()        \
    ReadWriteMode readWriteMode = {}; \
    readWriteMode.mode = *rip;        \
    rip += sizeof(char)               \

#define READ_WRITE_MODE_RAM readWriteMode.ram
#define READ_WRITE_MODE_REGISTER readWriteMode.reg
#define READ_WRITE_MODE_CONSTANT readWriteMode.constant

#define WRITE_VALUE_TO_RAM(address, value) ram[(address)] = (value)
#define READ_VALUE_FROM_RAM(index) ram[(index)]
#define READ_CHAR_FROM_RAM(index) (char) ram[(index)]

#define READ_CONSTANT() \
[this]()   \
{            \
    auto constant = *(constant_t *) this->rip;\
    this->rip += sizeof(constant_t);      \
    return constant;            \
}()

#define READ_OFFSET() \
[this]()   \
{            \
    auto offset = *(constant_t *) this->rip;\
    this->rip += sizeof(constant_t);      \
    return (size_t) offset;            \
}()

#define READ_INDEX_FROM_REGISTER() \
[this]()  \
{            \
    char regCode = *this->rip;\
    this->rip += sizeof(char); \
    return (size_t) this->registers[regCode];            \
}()

#define READ_VALUE_FROM_REGISTER() \
[this]()  \
{            \
    char regCode = *this->rip;\
    this->rip += sizeof(char); \
    return this->registers[regCode];            \
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
    /*
     * for (size_t i = 0; i < screenHeight; ++i) {
     *      for (size_t j = 0; j < screenWidth; ++j) printf("%c  ", READ_CHAR_FROM_RAM(i * screenWidth + j));
     *      putchar('\n');
     *      }
    */
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
    READ_READ_WRITE_MODE();

    if (READ_WRITE_MODE_RAM) {
        if (READ_WRITE_MODE_REGISTER & READ_WRITE_MODE_CONSTANT) {
            auto index = READ_INDEX_FROM_REGISTER();
            auto offset = READ_OFFSET();
            DATA_STACK_PUSH(READ_VALUE_FROM_RAM(index + offset));
        } else if (READ_WRITE_MODE_REGISTER & !READ_WRITE_MODE_CONSTANT) {
            auto index = READ_INDEX_FROM_REGISTER();
            DATA_STACK_PUSH(READ_VALUE_FROM_RAM(index));
        } else if (READ_WRITE_MODE_CONSTANT & !READ_WRITE_MODE_REGISTER) {
            auto offset = READ_OFFSET();
            DATA_STACK_PUSH(READ_VALUE_FROM_RAM(offset));
        }

        continue;
    }

    if (READ_WRITE_MODE_REGISTER & READ_WRITE_MODE_CONSTANT) {
        auto val = READ_VALUE_FROM_REGISTER();
        auto constant = READ_CONSTANT();
        DATA_STACK_PUSH(val + constant);
    } else if (READ_WRITE_MODE_REGISTER & !READ_WRITE_MODE_CONSTANT) {
        auto val = READ_VALUE_FROM_REGISTER();
        DATA_STACK_PUSH(val);
    } else if (READ_WRITE_MODE_CONSTANT & !READ_WRITE_MODE_REGISTER) {
        auto constant = READ_CONSTANT();
        DATA_STACK_PUSH(constant);
    }
})

DEFINE_COMMAND(pop, 5, false,
{
    READ_READ_WRITE_MODE();
    auto constant = DATA_STACK_POP();

    if (READ_WRITE_MODE_RAM) {
        if (READ_WRITE_MODE_REGISTER & READ_WRITE_MODE_CONSTANT) {
            auto index = READ_INDEX_FROM_REGISTER();
            auto offset = READ_OFFSET();
            WRITE_VALUE_TO_RAM(index + offset, constant);
        } else if (READ_WRITE_MODE_REGISTER & !READ_WRITE_MODE_CONSTANT) {
            auto index = READ_INDEX_FROM_REGISTER();
            WRITE_VALUE_TO_RAM(index, constant);
        } else if (READ_WRITE_MODE_CONSTANT & !READ_WRITE_MODE_REGISTER) {
            auto offset = READ_OFFSET();
            WRITE_VALUE_TO_RAM(offset, constant);
        }

        continue;
    }

    WRITE_VALUE_TO_REGISTER(constant);
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
    auto constant2 = DATA_STACK_POP();
    auto constant1 = DATA_STACK_POP();
    auto address = READ_ADDRESS();

    if (constant1 > constant2) {
        SET_ADDRESS(address);
    }
})

DEFINE_COMMAND(jae, 14, false,
{
    auto constant2 = DATA_STACK_POP();
    auto constant1 = DATA_STACK_POP();
    auto address = READ_ADDRESS();

    if (constant1 >= constant2) {
        SET_ADDRESS(address);
    }
})

DEFINE_COMMAND(jb, 15, false,
{
    auto constant2 = DATA_STACK_POP();
    auto constant1 = DATA_STACK_POP();
    auto address = READ_ADDRESS();

    if (constant1 < constant2) {
        SET_ADDRESS(address);
    }
})

DEFINE_COMMAND(jbe, 16, false,
{
    auto constant2 = DATA_STACK_POP();
    auto constant1 = DATA_STACK_POP();
    auto address = READ_ADDRESS();

    if (constant1 <= constant2) {
        SET_ADDRESS(address);
    }
})

DEFINE_COMMAND(je, 17, false,
{
    auto constant2 = DATA_STACK_POP();
    auto constant1 = DATA_STACK_POP();
    auto address = READ_ADDRESS();

    if (constant1 == constant2) {
        SET_ADDRESS(address);
    }
})

DEFINE_COMMAND(jne, 18, false,
{
    auto constant2 = DATA_STACK_POP();
    auto constant1 = DATA_STACK_POP();
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

DEFINE_COMMAND(sqrt, 21, true,
{
    auto constant = DATA_STACK_POP();
    DATA_STACK_PUSH(sqrt(constant));
})
