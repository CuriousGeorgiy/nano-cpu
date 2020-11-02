#ifndef STACK_HPP
#define STACK_HPP

#include "Canary.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

#ifndef NDEBUG
#define VERIFY_STACK validate()
#else
#define VERIFY_STACK
#endif

enum StackErrorCode {
    Ok,
    DeadLeftStackCanary,
    DeadRightStackCanary,
    DeadLeftDataCanary,
    DeadRightDataCanary,
    InvalidStackHashSum,
    NullPointerToData,
    SizeGreaterThanCapacity,
    CapacityEqualZero
};

struct StackError {
    StackErrorCode code;
    const char *description;
};

enum StackTransactionStatus {
    Begin,
    EndSuccess,
    EndFailure
};

template<typename T> class Stack {
public:
    typedef unsigned int hashSum_t;

    canary_t leftCanary;
    T *data;
    size_t size;
    size_t capacity;
    hashSum_t stackHashSum;
    canary_t rightCanary;

    T pop(bool *error);
    void push(T val);

    void evalHashSum();
    void print();
    void shrinkToFit();

    explicit Stack(size_t constructionCapacity = 10);

    Stack(const Stack &) = delete;
    Stack(Stack &&) = delete;

    Stack &operator=(const Stack &) = delete;
    Stack &operator=(Stack &&) = delete;

    ~Stack();
private:
    static constexpr double growCoefficientDefault = 2.5;
    static constexpr double growCoefficientIfFailure = 1.5;

    static std::FILE *dumpFile;

    StackError isValid();
    void validate();
    void transaction(StackTransactionStatus status);
    void dump(const StackError *stackError);
    void shrink();
    void grow(double growCoefficient);
};

template<typename T>
std::FILE *Stack<T>::dumpFile = std::fopen("StackDump.log", "w");

template<typename T>
Stack<T>::Stack(size_t constructionCapacity)
: leftCanary(CanaryValue), data((T *) callocBufWithBorderCanaries(constructionCapacity, sizeof(T))), size(0),
  capacity(constructionCapacity), stackHashSum(0), rightCanary(CanaryValue)
{
    assert(constructionCapacity > 0);

    evalHashSum();

    VERIFY_STACK;
}

template<typename T>
Stack<T>::~Stack()
{
    FREE_BUF_WITH_CANARY_BORDER(data);
}

template<typename T>
void Stack<T>::evalHashSum()
{
    const unsigned char *buffer = (const unsigned char *) this + sizeof(canary_t);
    stackHashSum = 0;

    auto rol = [](hashSum_t hashSum) {
        return (hashSum << 1u) | (hashSum >> (8 * sizeof(hashSum_t) - 1));
    };

    for (size_t i = 0; i < sizeof(Stack<T>) - sizeof(canary_t); ++i) {
        stackHashSum = rol(stackHashSum) + buffer[i];
    }
}

template<typename T>
StackError Stack<T>::isValid()
{
#define STACK_ERROR_WITH_DESCRIPTION(stackError) {(stackError), #stackError}

    if (leftCanary != CanaryValue) {
        return STACK_ERROR_WITH_DESCRIPTION(DeadLeftStackCanary);
    }

    if (rightCanary != CanaryValue) {
        return STACK_ERROR_WITH_DESCRIPTION(DeadRightStackCanary);
    }

    hashSum_t previousStackHashSum = stackHashSum;
    evalHashSum();
    if (previousStackHashSum != stackHashSum) {
        return STACK_ERROR_WITH_DESCRIPTION(InvalidStackHashSum);
    }

    if (leftBufCanary(data) != CanaryValue) {
        return STACK_ERROR_WITH_DESCRIPTION(DeadLeftDataCanary);
    }

    if (rightBufCanary(data, capacity * sizeof(T)) != CanaryValue) {
        return STACK_ERROR_WITH_DESCRIPTION(DeadRightDataCanary);
    }

    if (data == nullptr) {
        return STACK_ERROR_WITH_DESCRIPTION(NullPointerToData);
    }

    if (size > capacity) {
        return STACK_ERROR_WITH_DESCRIPTION(SizeGreaterThanCapacity);
    }

    if (capacity == 0) {
        return STACK_ERROR_WITH_DESCRIPTION(CapacityEqualZero);
    }

    return STACK_ERROR_WITH_DESCRIPTION(Ok);

#undef STACK_ERROR_WITH_DESCRIPTION
}

template<typename T>
void Stack<T>::dump(const StackError *stackError) {
    assert(stackError);

    switch (stackError->code) {
        case Ok: {
            fprintf(dumpFile,
                    "STACK_DUMP: status #%d, %s\n"
                    "Stack [%p]\n"
                    "{\n"
                    "\tsize     = %zu\n"
                    "\tcapacity = %zu\n"
                    "\tdata [%p]\n"
                    "\t{\n",
                    stackError->code, stackError->description, this, size, capacity, data);
            for (size_t i = 0; i < size; ++i) fprintf(dumpFile, "\t\t[%zu] = %lg\n", i, (double) data[i]);
            fprintf(dumpFile,
                    "\t}\n"
                    "}\n");
            return;
        }
        case DeadLeftStackCanary:
        case DeadRightStackCanary:
        case InvalidStackHashSum:
        case CapacityEqualZero:
        case NullPointerToData: {
            fprintf(dumpFile,
                    "STACK_DUMP: status #%d, %s\n"
                    "Stack [%p]\n"
                    "{\n"
                    "\tsize     = %zu\n"
                    "\tcapacity = %zu\n"
                    "\tdata [%p]\n"
                    "}\n",
                    stackError->code, stackError->description, this, size, capacity, data);
            return;
        }
        case DeadLeftDataCanary:
        case DeadRightDataCanary: {
            fprintf(dumpFile,
                    "STACK_DUMP: status #%d, %s\n"
                    "Stack [%p]\n"
                    "{\n"
                    "\tsize     = %zu\n"
                    "\tcapacity = %zu\n"
                    "\tdata [%p]\n"
                    "\t{\n",
                    stackError->code, stackError->description, this, size, capacity, data);
            for (size_t i = 0; i < size; ++i) fprintf(dumpFile, "\t\t[%zu] = %lg\n", i, (double) data[i]);
            fprintf(dumpFile,
                    "\t}\n"
                    "}\n");
            return;
        }
        case SizeGreaterThanCapacity: {
            fprintf(dumpFile,
                    "STACK_DUMP: status #%d, %s\n"
                    "Stack [%p]\n"
                    "{\n"
                    "\tsize     = %zu\n"
                    "\tcapacity = %zu\n"
                    "\tdata [%p]\n"
                    "\t{\n",
                    stackError->code, stackError->description, this, size, capacity, data);
            for (size_t i = 0; i < size; ++i) fprintf(dumpFile, "\t\t*[%10zu] = %lg\n", i, (double) data[i]);
            for (size_t i = size; i < capacity; ++i) fprintf(dumpFile, "\t\t[%10zu] = %lg\n", i, (double) data[i]);
            fprintf(dumpFile,
                    "\t}\n"
                    "}\n");
            return;
        }
    }
}

template<typename T>
void Stack<T>::validate() {
    StackError stackError = isValid();
    if (stackError.code != Ok) {
        dump(&stackError);
        exit(EXIT_FAILURE);
    }
}

template<typename T>
void Stack<T>::transaction(StackTransactionStatus status) {
    static Stack<T> *stackCopy = NULL;

    switch (status) {
        case Begin: {
            assert(stackCopy == nullptr);

            stackCopy = (Stack<T> *) std::calloc(1, sizeof(Stack<T>));
            stackCopy->data = (T *) callocBufWithBorderCanaries(capacity, sizeof(T));
            stackCopy->size = size;
            stackCopy->capacity = capacity;
            stackCopy->evalHashSum();

            std::memcpy(stackCopy->data, data, capacity * sizeof(T));

            break;
        }
        case EndSuccess: {
            assert(stackCopy != nullptr);

            stackCopy->~Stack();
            std::free(stackCopy);
            stackCopy = nullptr;

            break;
        }
        case EndFailure: {
            assert(stackCopy != nullptr);

            size = stackCopy->size;
            capacity = stackCopy->capacity;
            memcpy(data, stackCopy->data, stackCopy->capacity * sizeof(T));
            evalHashSum();

            stackCopy->~Stack();
            std::free(stackCopy);
            stackCopy = nullptr;

            break;
        }
    }
}

template<typename T>
void Stack<T>::grow(double growCoefficient) {
    VERIFY_STACK;
    assert(isfinite(growCoefficient));
    assert((growCoefficient == growCoefficientDefault) || (growCoefficient == growCoefficientIfFailure) ||
           (growCoefficient == 1));

    transaction(Begin);

    size_t newCapacity = (growCoefficient > 1) ? capacity * growCoefficient : capacity + 1;
    auto tmp = (T *) reallocBufWithBorderCanaries(data, newCapacity * sizeof(T));

    if ((tmp == NULL) && (growCoefficient == 1)) {
        transaction(EndFailure);
        VERIFY_STACK;
        return;
    }

    if (tmp == NULL) {
        if (growCoefficient == growCoefficientDefault) {
            transaction(EndSuccess);
            VERIFY_STACK;
            grow(growCoefficientIfFailure);
        } else {
            transaction(EndSuccess);
            VERIFY_STACK;
            grow(1);
        }
    }

    data = tmp;
    capacity = newCapacity;
    evalHashSum();

    transaction(EndSuccess);

    VERIFY_STACK;
}

template<typename T>
void Stack<T>::push(T val) {
    VERIFY_STACK;

    transaction(Begin);

    if (size < capacity) {
        data[size++] = val;
        evalHashSum();

        transaction(EndSuccess);
        VERIFY_STACK;
        return;
    }

    transaction(EndSuccess);
    grow(growCoefficientDefault);

    VERIFY_STACK;

    push(val);
}

template<typename T>
void Stack<T>::shrink() {
    VERIFY_STACK;

    transaction(Begin);

    size_t shrinkedCapacity = capacity / (growCoefficientDefault * growCoefficientDefault);

    if ((size > shrinkedCapacity) || (shrinkedCapacity == 0)) {
        transaction(EndSuccess);
        VERIFY_STACK;
        return;
    }

    auto tmp = (T *) reallocBufWithBorderCanaries(data, shrinkedCapacity * sizeof(T));

    if (tmp == NULL) {
        transaction(EndFailure);
        VERIFY_STACK;
        return;
    }

    data = tmp;
    capacity = shrinkedCapacity;
    evalHashSum();

    transaction(EndSuccess);
    VERIFY_STACK;
}

template<typename T>
T Stack<T>::pop(bool *error) {
    VERIFY_STACK;

    transaction(Begin);

    if (size == 0) {
        *error = true;
        transaction(EndFailure);
        VERIFY_STACK;
        return T();
    }

    T top = data[--size];
    evalHashSum();
    transaction(EndSuccess);

    shrink();

    VERIFY_STACK;

    return top;
}

template<typename T>
void Stack<T>::shrinkToFit() {
    VERIFY_STACK;

    transaction(Begin);

    auto tmp = (T *) reallocBufWithBorderCanaries(data, size * sizeof(T));

    if (tmp == NULL) {
        transaction(EndFailure);
        VERIFY_STACK;
        return;
    }

    data = tmp;
    capacity = size;
    evalHashSum();

    transaction(EndSuccess);

    VERIFY_STACK;
}

template<typename T>
void Stack<T>::print()
{
    VERIFY_STACK;

    StackError stackError = StackError{Ok, "Ok"};
    dump(&stackError);
}

#undef VERIFY_STACK

#endif /* STACK_HPP */
