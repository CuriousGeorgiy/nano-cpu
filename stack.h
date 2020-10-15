#ifndef STACK_H
#define STACK_H

#include "math.h"

/*!
 * Defines LIFO data structure
 */
struct Stack;

/*!
 * Defines the floating point type of elements stored in the stack
 */
typedef double val_t;

/*!
 * Defines invalid stack value
 */
const double STACK_POISON_VAL = NAN;

/*!
 * Dynamically allocates a Stack, calls the constructor and returns it
 *
 * @return pointer to Stack in case of success, NULL otherwise
 */
Stack *new_stack();

/*!
 * Pushes val into stack
 *
 * @param [in, out] stack pointer to Stack
 * @param [in] val value to be pushed
 *
 * @return 0 in case of success, a non-zero value otherwise
 *
 * @note In case of failure doesn't modify stack
 */
int stack_push(Stack **stack, val_t val);

/*!
 * Pops a value from stack. Check error before using the returned value
 *
 * @param [in, out] stack pointer to Stack
 * @param [in, out] error pointer to bool
 *
 * @return the popped value
 *
 * @note Returns STACK_POISON_VAL and sets error to true in case the stack was empty
 */
val_t stack_pop(Stack **stack, bool *error);

/*!
 * Shrinks stack to fit
 *
 * @param [in, out] stack pointer to Stack
 *
 * @return 0 in case of success, a non-zero value otherwise
 */
int stack_shrink_to_fit(Stack **stack);

/*!
 * Delete stack returned by new_stack
 *
 * @param [in, out] stack address of pointer to Stack
 */
void delete_stack(Stack **stack);

/*!
 * Prints stack to log file
 *
 * @param stack pointer to Stack
 */
void stack_print(const Stack *stack);

#endif /* STACK_H */
