#include "Processor.hpp"

#include "ReadWrite.hpp"

#include <cassert>

Processor::Processor(const char *assembly, size_t assemblySize)
: dataStack(), callStack(), assembly(assembly), rip(assembly), assemblySize(assemblySize), registers{0, 0, 0}
{
    assert(assembly != nullptr);
}

void Processor::operator()()
{
    while (rip - assembly < assemblySize) {
        switch (*rip) {
#define DEFINE_COMMAND(name, code, noArg, processorSrc) \
        case code: {                                    \
            rip += sizeof(char);                        \
                                                        \
            {                                           \
                processorSrc                            \
            }                                           \
                                                        \
            break;                                      \
        }
#include "Commands.hpp"
#undef DEFINE_COMMAND
        }
    }
}
