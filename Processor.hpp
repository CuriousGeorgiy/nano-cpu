#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "Stack.hpp"
#include "ReadWrite.hpp"

class Processor {
public:
    explicit Processor(const char *inputFileName);

    Processor(const Processor &) = delete;
    Processor(Processor &&) = delete;

    Processor &operator=(const Processor &) = delete;
    Processor &operator=(Processor &&) = delete;

    ~Processor();

    void operator()();
private:
    Stack<constant_t> dataStack;
    Stack<ptrdiff_t> callStack;
    char *assembly;
    const char *rip;
    size_t assemblySize;
    constant_t registers[7];
    constant_t ram[2500];
};

#endif /* PROCESSOR_HPP */
