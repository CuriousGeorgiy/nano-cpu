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
    static const size_t nRegisters = 7;
    static const size_t ramSize = 2500;
    static const size_t screenWidth = 50;
    static const size_t screenHeight = 50;

    Stack<constant_t> dataStack;
    Stack<ptrdiff_t> callStack;
    char *assembly;
    const char *rip;
    size_t assemblySize;
    constant_t registers[nRegisters];
    constant_t ram[ramSize];
};

#endif /* PROCESSOR_HPP */
