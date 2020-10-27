#ifndef STACK_HPP
#define STACK_HPP

#include "canary.h"
#include "log.h"
#include "memory_alloc.h"

#include <cassert>
#include <cmath>
#include <cstring>

#ifndef NDEBUG
#define VERIFY_STACK() verify()
#else
#define VERIFY_STACK(stack)
#endif

const double growCoefficientDefault = 2.5;
const double growCoefficientIfFailure = 1.5;

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

enum TransactionStatus {
    Begin,
    EndSuccess,
    EndFailure
};

/*!
 * Defines LIFO data structure
 */
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
    StackError isCorrect();
    void verify();
    void transaction(TransactionStatus status);
    void dump(const StackError *stackError);
    void shrink();
    void grow(double growCoefficient);
};

template<typename T>
Stack<T>::Stack(size_t constructionCapacity)
: leftCanary(CANARY_VALUE), data((T *) calloc_with_border_canaries(constructionCapacity, sizeof(T))), size(0),
  capacity(constructionCapacity), stackHashSum(0), rightCanary(CANARY_VALUE)
{
    assert(constructionCapacity > 0);

    evalHashSum();

    VERIFY_STACK();
}

template<typename T>
Stack<T>::~Stack()
{
    FREE_WITH_CANARY_BORDER(data);
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
StackError Stack<T>::isCorrect()
{
#define STACK_ERROR_WITH_DESCRIPTION(stackError) {stackError, #stackError}

    if (leftCanary != CANARY_VALUE) {
        return STACK_ERROR_WITH_DESCRIPTION(DeadLeftStackCanary);
    }

    if (rightCanary != CANARY_VALUE) {
        return STACK_ERROR_WITH_DESCRIPTION(DeadRightStackCanary);
    }

    hashSum_t previousStackHashSum = stackHashSum;
    evalHashSum();
    if (previousStackHashSum != stackHashSum) {
        return STACK_ERROR_WITH_DESCRIPTION(InvalidStackHashSum);
    }

    if ((left_canary(data) != CANARY_VALUE)) {
        return STACK_ERROR_WITH_DESCRIPTION(DeadLeftDataCanary);
    }

    if ((right_canary(data, capacity * sizeof(T)) != CANARY_VALUE)) {
        return STACK_ERROR_WITH_DESCRIPTION(DeadRightDataCanary);
    }

    if (data == NULL) {
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
            logging("STACK_DUMP: status #%d, %s\n"
                    "Stack [%p]\n"
                    "{\n"
                    "\tsize     = %zu\n"
                    "\tcapacity = %zu\n"
                    "\tdata [%p]\n"
                    "\t{\n", stackError->code, stackError->description, this, size, capacity, data);
            for (size_t i = 0; i < size; ++i) logging("\t\t[%zu] = %lg\n", i, (double) data[i]);
            logging("\t}\n"
                    "}\n");
            return;
        }
        case DeadLeftStackCanary:
        case DeadRightStackCanary:
        case InvalidStackHashSum:
        case CapacityEqualZero:
        case NullPointerToData: {
            logging("STACK_DUMP: status #%d, %s\n"
                    "Stack [%p]\n"
                    "{\n"
                    "\tsize     = %zu\n"
                    "\tcapacity = %zu\n"
                    "\tdata [%p]\n"
                    "}\n", stackError->code, stackError->description, this, size, capacity, data);
            return;
        }
        case DeadLeftDataCanary:
        case DeadRightDataCanary: {
            logging("STACK_DUMP: status #%d, %s\n"
                    "Stack [%p]\n"
                    "{\n"
                    "\tsize     = %zu\n"
                    "\tcapacity = %zu\n"
                    "\tdata [%p]\n"
                    "\t{\n", stackError->code, stackError->description, this, size, capacity, data);
            for (size_t i = 0; i < size; ++i) logging("\t\t[%zu] = %lg\n", i, (double) data[i]);
            logging("\t}\n"
                    "}\n");
            return;
        }
        case SizeGreaterThanCapacity: {
            logging("STACK_DUMP: status #%d, %s\n"
                    "Stack [%p]\n"
                    "{\n"
                    "\tsize     = %zu\n"
                    "\tcapacity = %zu\n"
                    "\tdata [%p]\n"
                    "\t{\n", stackError->code, stackError->description, this, size, capacity, data);
            for (size_t i = 0; i < size; ++i) logging("\t\t*[%10zu] = %lg\n", i, (double) data[i]);
            for (size_t i = size; i < capacity; ++i) logging("\t\t[%10zu] = %lg\n", i, (double) data[i]);
            logging("\t}\n"
                    "}\n");
            return;
        }
    }
}

template<typename T>
void Stack<T>::verify() {
    StackError stackError = isCorrect();
    if (stackError.code) {
        dump(&stackError);
        exit(EXIT_FAILURE);
    }
}

template<typename T>
void Stack<T>::transaction(TransactionStatus status) {
    static Stack<T> *stackCopy = NULL;

    switch (status) {
        case Begin: {
            assert(stackCopy == nullptr);

            stackCopy = (Stack<T> *) std::calloc(1, sizeof(Stack<T>));
            stackCopy->data = (T *) calloc_with_border_canaries(capacity, sizeof(T));
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
    VERIFY_STACK();
    assert(isfinite(growCoefficient));
    assert((growCoefficient == growCoefficientDefault) || (growCoefficient == growCoefficientIfFailure) ||
           (growCoefficient == 1));

    transaction(Begin);

    size_t newCapacity = (growCoefficient > 1) ? capacity * growCoefficient : capacity + 1;
    auto tmp = (T *) realloc_with_border_canaries(data, newCapacity * sizeof(T));

    if ((tmp == NULL) && (growCoefficient == 1)) {
        transaction(EndFailure);
        VERIFY_STACK();
        return;
    }

    if (tmp == NULL) {
        if (growCoefficient == growCoefficientDefault) {
            transaction(EndSuccess);
            VERIFY_STACK();
            grow(growCoefficientIfFailure);
        } else {
            transaction(EndSuccess);
            VERIFY_STACK();
            grow(1);
        }
    }

    data = tmp;
    capacity = newCapacity;
    evalHashSum();

    transaction(EndSuccess);

    VERIFY_STACK();
}

template<typename T>
void Stack<T>::push(T val) {
    VERIFY_STACK();

    transaction(Begin);

    if (size < capacity) {
        data[size++] = val;
        evalHashSum();

        transaction(EndSuccess);
        VERIFY_STACK();
        return;
    }

    transaction(EndSuccess);
    grow(growCoefficientDefault);

    VERIFY_STACK();

    push(val);
}

template<typename T>
void Stack<T>::shrink() {
    VERIFY_STACK();

    transaction(Begin);

    size_t shrinkedCapacity = capacity / (growCoefficientDefault * growCoefficientDefault);

    if ((size > shrinkedCapacity) || (shrinkedCapacity == 0)) {
        transaction(EndSuccess);
        VERIFY_STACK();
        return;
    }

    auto tmp = (T *) realloc_with_border_canaries(data, shrinkedCapacity * sizeof(T));

    if (tmp == NULL) {
        transaction(EndFailure);
        VERIFY_STACK();
        return;
    }

    data = tmp;
    capacity = shrinkedCapacity;
    evalHashSum();

    transaction(EndSuccess);
    VERIFY_STACK();
}

template<typename T>
T Stack<T>::pop(bool *error) {
    VERIFY_STACK();

    transaction(Begin);

    if (size == 0) {
        *error = true;
        transaction(EndFailure);
        VERIFY_STACK();
        return T();
    }

    T top = data[--size];
    evalHashSum();
    transaction(EndSuccess);

    shrink();

    VERIFY_STACK();

    return top;
}

template<typename T>
void Stack<T>::shrinkToFit() {
    VERIFY_STACK();

    transaction(Begin);

    auto tmp = (T *) realloc_with_border_canaries(data, size * sizeof(T));

    if (tmp == NULL) {
        transaction(EndFailure);
        VERIFY_STACK();
        return;
    }

    data = tmp;
    capacity = size;
    evalHashSum();

    transaction(EndSuccess);

    VERIFY_STACK();
}

template<typename T>
void Stack<T>::print()
{
    VERIFY_STACK();

    StackError stackError = StackError{Ok, "Ok"};
    dump(&stackError);
}

#undef VERIFY_STACK
#undef VERIFY_STACK_AND_RETURN

#endif /* STACK_HPP */
