#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "Stack.hpp"
#include "ReadWrite.hpp"

class Processor {
public:
    Processor(const char *assembly, size_t assemblySize);

    Processor(const Processor &) = delete;
    Processor(Processor &&) = delete;

    Processor &operator=(const Processor &) = delete;
    Processor &operator=(Processor &&) = delete;

    ~Processor() = default;

    void operator()();
private:
    Stack<constant_t> dataStack;
    Stack<ptrdiff_t> callStack;
    const char *assembly;
    const char *rip;
    const size_t assemblySize;
    double registers[7];
};

#endif /* PROCESSOR_HPP */
