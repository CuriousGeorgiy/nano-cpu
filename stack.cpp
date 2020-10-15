#include "stack.h"

#include "canary.h"
#include "error.h"
#include "hash_sum.h"
#include "log.h"
#include "memory_alloc.h"

#include <assert.h>
#include <memory.h>
#include <stdlib.h>

static const size_t CONSTRUCTION_CAPACITY = 1;

static const double GROW_COEFFICIENT = 2.5;

static const double GROW_COEFFICIENT_IF_FAILURE = 1.5;

struct Stack {
    val_t *data;
    size_t size;
    size_t capacity;

    hash_sum_t hash_sum;
};

enum stack_error_code {
    OK,
    DEAD_STACK_CANARY,
    DEAD_DATA_CANARY,
    INVALID_HASH_SUM,
    NULL_POINTER_TO_STACK,
    NULL_POINTER_TO_DATA,
    INVALID_DATA,
    SIZE_GREATER_THAN_CAPACITY,
    CAPACITY_EQUAL_ZERO
};

struct StackError {
    stack_error_code code;
    const char *description;
};

enum transaction_status {
    BEGIN,
    END_SUCCESS,
    END_FAILURE
};

#ifndef NDEBUG
#define VERIFY_STACK(stack) stack_verify(stack)

#define VERIFY_STACK_AND_RETURN(stack, val) do {                     \
                                                VERIFY_STACK(stack); \
                                                return val;          \
                                            } while (0)
#else
#define VERIFY_STACK(stack)
#define VERIFY_STACK_AND_RETURN(stack, val) return val
#endif

hash_sum_t stack_eval_hash_sum(const Stack *stack) {
    assert(stack);

    return eval_hash_sum((const unsigned char *) stack, sizeof(Stack) - sizeof(hash_sum_t));
}

bool stack_validate_data(const Stack *stack) {
    assert(stack != NULL);

    for (size_t i = 0; i < stack->size; ++i) {
        if (!isfinite(stack->data[i])) return false;
    }

    for (size_t i = stack->size; i < stack->capacity; ++i) {
        if (isfinite(stack->data[i])) return false;
    }

    return true;
}

StackError stack_error(const Stack *stack) {
#define STACK_ERROR_WITH_DESCRIPTION(error) {error, #error}

    if (stack == NULL) {
        return STACK_ERROR_WITH_DESCRIPTION(NULL_POINTER_TO_STACK);
    }

    if ((left_canary(stack) != CANARY_VALUE) || (right_canary(stack, sizeof(Stack)) != CANARY_VALUE)) {
        return STACK_ERROR_WITH_DESCRIPTION(DEAD_STACK_CANARY);
    }

    if (stack->hash_sum != stack_eval_hash_sum(stack)) {
        return STACK_ERROR_WITH_DESCRIPTION(INVALID_HASH_SUM);
    }

    if ((left_canary(stack->data) != CANARY_VALUE)) {
        return STACK_ERROR_WITH_DESCRIPTION(DEAD_DATA_CANARY);
    }

    canary_t y = left_canary(stack->data);
    canary_t x = right_canary(stack->data, stack->capacity * sizeof(val_t));

    if ((right_canary(stack->data, stack->capacity * sizeof(val_t)) != CANARY_VALUE)) {
        return STACK_ERROR_WITH_DESCRIPTION(DEAD_DATA_CANARY);
    }

    if (stack->data == NULL) {
        return STACK_ERROR_WITH_DESCRIPTION(NULL_POINTER_TO_DATA);
    }

    if (!stack_validate_data(stack)) {
        return STACK_ERROR_WITH_DESCRIPTION(INVALID_DATA);
    }

    if (stack->size > stack->capacity) {
        return STACK_ERROR_WITH_DESCRIPTION(SIZE_GREATER_THAN_CAPACITY);
    }

    if (!stack->capacity) {
        return STACK_ERROR_WITH_DESCRIPTION(CAPACITY_EQUAL_ZERO);
    }

    return STACK_ERROR_WITH_DESCRIPTION(OK);

#undef STACK_ERROR_WITH_DESCRIPTION
}

void stack_dump(const Stack *stack, const StackError *stackerror) {
    assert(stackerror);
    rol(4);

    switch (stackerror->code) {
        case OK: {
            logging("STACK_DUMP: status #%d, %s\n"
                "Stack [%p]\n"
                "{\n"
                "\tsize     = %zu\n"
                "\tcapacity = %zu\n"
                "\tdata [%p]\n"
                "\t{\n", stackerror->code, stackerror->description, stack, stack->size, stack->capacity, stack->data);
            for (size_t i = 0; i < stack->size; ++i) logging("\t\t[%zu] = %f\n", i, stack->data[i]);
            logging("\t}\n"
                "}\n");
            return;
        }
        case DEAD_STACK_CANARY:
        case INVALID_HASH_SUM:
        case CAPACITY_EQUAL_ZERO:
        case NULL_POINTER_TO_DATA: {
            logging("STACK_DUMP: status #%d, %s\n"
                "Stack [%p]\n"
                "{\n"
                "\tsize     = %zu\n"
                "\tcapacity = %zu\n"
                "\tdata [%p]\n"
                "}\n", stackerror->code, stackerror->description, stack, stack->size, stack->capacity, stack->data);
            return;
        }
        case NULL_POINTER_TO_STACK: {
            logging("STACK_DUMP: status #%d, %s\n"
                "Stack [%p]\n", stackerror->code, stackerror->description, stack);
            return;
        }
        case DEAD_DATA_CANARY:
        case INVALID_DATA: {
            logging("STACK_DUMP: status #%d, %s\n"
                "Stack [%p]\n"
                "{\n"
                "\tsize     = %zu\n"
                "\tcapacity = %zu\n"
                "\tdata [%p]\n"
                "\t{\n", stackerror->code, stackerror->description, stack, stack->size, stack->capacity, stack->data);
            for (size_t i = 0; i < stack->size; ++i) logging("\t\t[%zu] = %f\n", i, stack->data[i]);
            logging("\t}\n"
                "}\n");
            return;
        }
        case SIZE_GREATER_THAN_CAPACITY: {
            logging("STACK_DUMP: status #%d, %s\n"
                "Stack [%p]\n"
                "{\n"
                "\tsize     = %zu\n"
                "\tcapacity = %zu\n"
                "\tdata [%p]\n"
                "\t{\n", stackerror->code, stackerror->description, stack, stack->size, stack->capacity, stack->data);
            for (size_t i = 0; i < stack->size; ++i) logging("\t\t*[%10zu] = %f\n", i, stack->data[i]);
            for (size_t i = stack->size; i < stack->capacity; ++i) logging("\t\t[%10zu] = %f\n", i, stack->data[i]);
            logging("\t}\n"
                "}\n");
            return;
        }
    }
}

void stack_verify(const Stack *stack) {
    StackError stackerror = stack_error(stack);

    if (stackerror.code) {
        stack_dump(stack, &stackerror);
        abort();
    }
}

void struct_transaction_rollback(Stack **stack, Stack *stack_copy) {
    assert(stack_copy != NULL);

    delete_stack(stack);
    *stack = stack_copy;
}

int struct_transaction(Stack **stack, transaction_status status) {
    assert(stack != NULL);

    static Stack *stack_copy = NULL;

    switch (status) {
        case BEGIN: {
            if (stack_copy != NULL) {
                ERROR_OCCURRED_IN_FUNC(struct_transaction, "started new transaction before ending previous");
                return -1;
            }

            stack_copy = (Stack *) calloc_with_border_canaries(1, sizeof(Stack));

            if (stack_copy == NULL) {
                ERROR_OCCURRED_CALLING(calloc, "returned NULL");
                return -1;
            }

            stack_copy->data = (val_t *) calloc_with_border_canaries((*stack)->capacity, sizeof(val_t));

            if (stack_copy->data == NULL) {
                ERROR_OCCURRED_CALLING(calloc, "returned NULL");
                return -1;
            }

            stack_copy->size = (*stack)->size;
            stack_copy->capacity = (*stack)->capacity;
            stack_copy->hash_sum = stack_eval_hash_sum(stack_copy);

            memcpy(stack_copy->data, (*stack)->data, (*stack)->capacity * sizeof(val_t));

            return 0;
        }
        case END_SUCCESS: {
            assert(stack_copy != NULL);

            delete_stack(&stack_copy);

            return 0;
        }
        case END_FAILURE: {
            assert(stack_copy != NULL);

            struct_transaction_rollback(stack, stack_copy);
            stack_copy = NULL;

            return 0;
        }
    }
}

void stack_poison_empty_data(Stack *stack) {
    for (size_t i = stack->size; i < stack->capacity; ++i) {
        stack->data[i] = STACK_POISON_VAL;
    }
}

int stack_ctor(Stack *stack) {
    assert(CONSTRUCTION_CAPACITY > 0);

    stack->data = (val_t *) calloc_with_border_canaries(CONSTRUCTION_CAPACITY, sizeof(val_t));

    if (stack->data == NULL) {
        ERROR_OCCURRED_CALLING(calloc_with_border_canaries, "returned NULL");
        return 1;
    }

    stack->capacity = CONSTRUCTION_CAPACITY;
    stack->size = 0;
    stack->hash_sum = stack_eval_hash_sum(stack);

    stack_poison_empty_data(stack);

    VERIFY_STACK_AND_RETURN(stack, 0);
}

Stack *new_stack() {
    Stack *stack = (Stack *) calloc_with_border_canaries(1, sizeof(Stack));

    if (stack == NULL) {
        ERROR_OCCURRED_CALLING(calloc_with_border_canaries, "returned NULL");
        return NULL;
    }

    if (stack_ctor(stack)) {
        ERROR_OCCURRED_CALLING(stack_ctor, "returned non-zero value");
        return NULL;
    }

    return stack;
}

int stack_grow(Stack **stack, double grow_coefficient) {
    VERIFY_STACK(*stack);
    assert(isfinite(grow_coefficient));
    assert((grow_coefficient == GROW_COEFFICIENT) || (grow_coefficient == GROW_COEFFICIENT_IF_FAILURE) ||
           (grow_coefficient == 1));

    struct_transaction(stack, BEGIN);

    size_t new_capacity = (grow_coefficient > 1) ? (*stack)->capacity * GROW_COEFFICIENT : (*stack)->capacity + 1;
    val_t *tmp = (val_t *) realloc_with_border_canaries((*stack)->data, new_capacity * sizeof(val_t));

    if ((tmp == NULL) && (grow_coefficient == 1)) {
        ERROR_OCCURRED_CALLING(realloc_with_border_canaries, "returned NULL");
        struct_transaction(stack, END_FAILURE);
        VERIFY_STACK_AND_RETURN(*stack, 1);
    }

    if (tmp == NULL) {
        if (grow_coefficient == GROW_COEFFICIENT) {
            struct_transaction(stack, END_SUCCESS);
            VERIFY_STACK_AND_RETURN((*stack), stack_grow(stack, GROW_COEFFICIENT_IF_FAILURE));
        } else {
            struct_transaction(stack, END_SUCCESS);
            VERIFY_STACK_AND_RETURN(*stack, stack_grow(stack, 1));
        }
    }

    (*stack)->data = tmp;
    (*stack)->capacity = new_capacity;
    (*stack)->hash_sum = stack_eval_hash_sum(*stack);

    stack_poison_empty_data((*stack));

    struct_transaction(stack, END_SUCCESS);
    VERIFY_STACK_AND_RETURN(*stack, 0);
}

int stack_push(Stack **stack, val_t val) {
    VERIFY_STACK(*stack);
    assert(isfinite(val));

    if (struct_transaction(stack, BEGIN)) {
        ERROR_OCCURRED_CALLING(struct_transaction, "failed to begin");
        return 1;
    }

    if ((*stack)->size < (*stack)->capacity) {
        (*stack)->data[(*stack)->size++] = val;
        (*stack)->hash_sum = stack_eval_hash_sum((*stack));

        struct_transaction(stack, END_SUCCESS);
        VERIFY_STACK_AND_RETURN(*stack, 0);
    }

    struct_transaction(stack, END_SUCCESS);
    if (stack_grow(stack, GROW_COEFFICIENT)) {
        ERROR_OCCURRED_CALLING(stack_grow, "returned non-zero value");
        VERIFY_STACK_AND_RETURN(*stack, 1);
    }
    
    VERIFY_STACK_AND_RETURN(*stack, stack_push(stack, val));
}

int stack_shrink(Stack **stack) {
    VERIFY_STACK(*stack);

    if (struct_transaction(stack, BEGIN)) {
        ERROR_OCCURRED_CALLING(struct_transaction, "failed to begin");
        return 1;
    }

    size_t shrinked_capacity = (*stack)->capacity / (GROW_COEFFICIENT * GROW_COEFFICIENT);

    if ((((*stack)->size) > shrinked_capacity) || (shrinked_capacity == 0)) {
        struct_transaction(stack, END_SUCCESS);
        VERIFY_STACK_AND_RETURN((*stack), 0);
    }

    val_t *tmp = (val_t *) realloc_with_border_canaries((*stack)->data, shrinked_capacity * sizeof(val_t));

    if (tmp == NULL) {
        ERROR_OCCURRED_CALLING(realloc_with_border_canaries, "returned NULL");
        struct_transaction(stack, END_FAILURE);
        VERIFY_STACK_AND_RETURN(*stack, 1);
    }

    printf("yay! (*stack) was shrinked from capacity %zu to %zu\n", (*stack)->capacity, shrinked_capacity);

    (*stack)->data = tmp;
    (*stack)->capacity = shrinked_capacity;
    (*stack)->hash_sum = stack_eval_hash_sum((*stack));

    struct_transaction(stack, END_SUCCESS);
    VERIFY_STACK_AND_RETURN(*stack, 0);
}

val_t stack_pop(Stack **stack, bool *error) {
    VERIFY_STACK(*stack);

    if (struct_transaction(stack, BEGIN)) {
        ERROR_OCCURRED_CALLING(struct_transaction, "failed to begin");
        return 1;
    }

    if (!(*stack)->size) {
        *error = true;
        struct_transaction(stack, END_FAILURE);
        VERIFY_STACK_AND_RETURN(*stack, STACK_POISON_VAL);
    }

    val_t top = (*stack)->data[--(*stack)->size];
    (*stack)->hash_sum = stack_eval_hash_sum((*stack));
    (*stack)->data[(*stack)->size] = STACK_POISON_VAL;
    struct_transaction(stack, END_SUCCESS);

    if (stack_shrink(stack)) {
        ERROR_OCCURRED_CALLING(stack_shrink_to_fit, "returned non-zero value");
    }

    VERIFY_STACK_AND_RETURN(*stack, top);
}

int stack_shrink_to_fit(Stack **stack) {
    VERIFY_STACK(*stack);

    if (struct_transaction(stack, BEGIN)) {
        ERROR_OCCURRED_CALLING(struct_transaction, "failed to begin");
        return 1;
    }

    val_t *tmp = (val_t *) realloc_with_border_canaries((*stack)->data, (*stack)->size * sizeof(val_t));

    if (tmp == NULL) {
        ERROR_OCCURRED_CALLING(realloc_with_border_canaries, "returned NULL");
        struct_transaction(stack, END_FAILURE);
        VERIFY_STACK_AND_RETURN(*stack, 1);
    }

    (*stack)->data = tmp;
    (*stack)->capacity = (*stack)->size;
    (*stack)->hash_sum = stack_eval_hash_sum((*stack));

    struct_transaction(stack, END_SUCCESS);
    VERIFY_STACK_AND_RETURN(*stack, 0);
}

void stack_dtor(Stack *stack) {
    VERIFY_STACK(stack);

    FREE_WITH_CANARY_BORDER(stack->data);
}

void delete_stack(Stack **stack) {
    VERIFY_STACK(*stack);

    stack_dtor(*stack);

    FREE_WITH_CANARY_BORDER(*stack);
}

void stack_print(const Stack *stack)
{
    VERIFY_STACK(stack);

    StackError stackerror = StackError{OK, "OK"};

    stack_dump(stack, &stackerror);
}

#undef VERIFY_STACK
#undef VERIFY_STACK_AND_RETURN