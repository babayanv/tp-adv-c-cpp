/*
Бабаян Владимир - АПО-12

# Задача B-4. Парсер с длинной арифметикой
**Time limit:**   14 s
**Memory limit:** 64 M

Требуется написать программу, которая способна вычислять арифметические выражения.
Выражения могут содержать:
1) Знаки операций '+', '-', '/', '*';
2) Скобки '(', ')';
3) Целые числа, в нотации '123', в том числе они достаточно большие (требуется реализовать длинную арифметику), все операции должны быть целочисленные;
4) Необходимо учитывать приоритеты операций, и возможность унарного минуса, пробелы ничего не значат;
5) Если в выражении встретилась ошибка требуется вывести в стандартный поток вывода "[error]" (без кавычек).

**ВАЖНО! Программа в любом случае должна возвращать 0. Не пишите return -1, exit(1) и т.п. Даже если обнаружилась какая-то ошибка, все равно необходимо вернуть 0! (и напечатать [error] в stdout).**

### Examples
| Input                                  | Output                         |
| :------------------------------------: | :----------------------------: |
| `(1+2)*3353454445445345434545435647/6` | `1676727222722672717272717823` |
*/
#define _XOPEN_SOURCE 700


#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>



#define BASE_DENARY        1000000000 // 10^9
#define MAX_UINT32_LENGTH  9
#define BIGINT_STR_GROWTH  64
#define TOKEN_STACK_GROWTH 8

#define LOG_ERROR fprintf(stdout, "[error]\n");



////////////////////////////////////////////////////////////////////////////////
// Custom types
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////
//// Bigint
////////////////////////////////////////
typedef enum
{
    NEGATIVE = -1,
    POSITIVE = 1
} sign_t;



typedef struct
{
    sign_t sign;
    uint32_t *value;
    size_t length;
    size_t capacity;
} bigint;



////////////////////////////////////////
//// Token
////////////////////////////////////////
typedef enum
{
    NONE = 0,
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    LEFT_PARENTHESIS,
    RIGHT_PARENTHESIS,
    NUMBER,
    END_OF_EXPRETION,
    END_OF_FILE
} token_kind;



typedef bigint* (*operation_fn_t)(const bigint*, const bigint*);



typedef union
{
    bigint *bigint;
    operation_fn_t operation_fn;
} token_data;



typedef struct
{
    token_kind kind;
    token_data data;
} token_t;



////////////////////////////////////////
//// Token Stack
////////////////////////////////////////
typedef struct
{
    token_t *array;
    size_t length;
    size_t capacity;
} token_stack;



////////////////////////////////////////
//// Postfix Notation
////////////////////////////////////////
typedef struct
{
    token_stack *operands;
    token_stack *operators;
} postfix_notation;



////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////
//// General
////////////////////////////////////////
//! Performs mathematical calculation according to passed string.
bigint* calculate(const char *src_str, size_t str_length);
size_t max(size_t a, size_t b);



////////////////////////////////////////
//// The bigint interface
////////////////////////////////////////
bigint* bigint_from_string(const char *src_str, size_t str_length);
bigint* bigint_from_int64(int64_t src_val);
void bigint_destr(bigint **origin);
char* bigint_to_string(const bigint *src_bint);
bool bigint_print(FILE *stream, const bigint *src_bint);

static bigint* bigint_create_empty(size_t cap);
static bool bigint_pushback(bigint *dest_bint, uint32_t src_val);
static bool bigint_pushfront(bigint *dest_bint, uint32_t src_val);
static bool bigint_expand(bigint *origin, size_t new_cap);
static void bigint_remove_leading_zeros(bigint *origin);

int bigint_compare(const bigint *left, const bigint *right);

//! Next functions perform mathematical operations between two bigints and write
//  the result into a newly allocated instance of bigint which has to be freed
//  manually. Considering the combinations of the signs of passed bigints, these
//  call a corresponding function from the set below.
bigint* bigint_add(const bigint *left, const bigint *right);
bigint* bigint_subtract(const bigint *left, const bigint *right);
bigint* bigint_multiply(const bigint *left, const bigint *right);
bigint* bigint_divide(const bigint *left, const bigint *right);

//! Next functions perform mathematical operations between absolute values of
//  two bigints and write the result into existing binint instance. These are
//  called exclusevly by the functions from the set above.
static bool bigint_add_values(const bigint *left, const bigint *right, bigint *result);
static bool bigint_subtract_values(const bigint *left, const bigint *right, bigint *result);
static bool bigint_multiply_values(const bigint *left, const bigint *right, bigint *result);
static bool bigint_divide_values(const bigint *left, const bigint *right, bigint *result);



////////////////////////////////////////
//// The token interface
////////////////////////////////////////
token_t* token_get(const char *src_str, size_t *str_iter);
void token_destr(token_t **origin);

static bool token_get_operator(char literal, token_t *dest_token);
static bool token_get_operand(const char *src_str, size_t *str_iter, token_t *dest_token);


////////////////////////////////////////
//// The token_stack interface
////////////////////////////////////////
token_stack* token_stack_create(size_t cap);
void token_stack_destr(token_stack **origin);
bool token_stack_push(token_stack *dest, const token_t *value);
token_t token_stack_pop(token_stack *origin);
const token_t* token_stack_peek(const token_stack *origin);

static bool token_stack_expand(token_stack *origin, size_t cap);
static bool token_stack_is_empty(const token_stack *src);
static bool token_stack_is_full(const token_stack *src);



////////////////////////////////////////
//// The postfix_notation interface
////////////////////////////////////////
postfix_notation* postfix_notation_create_empty();
void postfix_notation_destr(postfix_notation **origin);
bool postfix_notation_add_token(postfix_notation *dest, token_t *token);

static int postfix_notation_prioritize(const token_t *token);
static bool postfix_notation_can_reduce(const postfix_notation *origin, const token_t *next_token);
static bool postfix_notation_reduce(postfix_notation *origin);



////////////////////////////////////////////////////////////////////////////////
// Entry point.
////////////////////////////////////////////////////////////////////////////////
int main()
{
    while(!feof(stdin))
    {
        char *input_str = NULL;
        size_t size = 0;
        long int input_length = getline(&input_str, &size, stdin);
        if(!input_str)
        {
            LOG_ERROR;
            return 0;
        }
        if(input_length == -1)
        {
            free(input_str);
            return 0;
        }

        bigint *result = calculate(input_str, (size_t)input_length);
        if(!result)
        {
            LOG_ERROR;
            free(input_str);
            return 0;
        }

        bigint_print(stdout, result);

        free(input_str);
        bigint_destr(&result);
    }

    return 0;
}



////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////
//// General
////////////////////////////////////////

//! Performs mathematical calculation according to passed string
bigint* calculate(const char *src_str, size_t str_length)
{
    if(!src_str)
    {
        return NULL;
    }

    postfix_notation *notation = postfix_notation_create_empty();
    if(!notation)
    {
        return NULL;
    }

    token_kind prev = LEFT_PARENTHESIS; // for unary minus detection
    for(size_t str_iter = 0; str_iter <= str_length; )
    {
        token_t *token = token_get(src_str, &str_iter);
        if(!token)
        {
            postfix_notation_destr(&notation);
            return NULL;
        }

        // Add fictive 0 as a left operand for unary minus as 0 - num == -num
        if(prev == LEFT_PARENTHESIS && token->kind == SUBTRACTION)
        {
            token_t tzero = { .kind = NUMBER, .data.bigint = bigint_from_int64(0) };
            if(!tzero.data.bigint)
            {
                token_destr(&token);
                postfix_notation_destr(&notation);
                return NULL;
            }

            if(!token_stack_push(notation->operands, &tzero))
            {
                token_destr(&token);
                postfix_notation_destr(&notation);
                return NULL;
            }
        }

        if(!postfix_notation_add_token(notation, token))
        {
            postfix_notation_destr(&notation);
            token_destr(&token);
            return NULL;
        }

        prev = token->kind;

        free(token);
    }

    // If the passed string is a correct and valid mathematical expression,
    // the only value in the notation left is the result.
    bigint *result = token_stack_pop(notation->operands).data.bigint;

    // If there are no values in the operands stack, there was none in the source expression;
    // If there are any operators or operands left other than the result, the result is undefined.
    if(!result || !token_stack_is_empty(notation->operators) || !token_stack_is_empty(notation->operands))
    {
        postfix_notation_destr(&notation);
        bigint_destr(&result);
        return NULL;
    }

    postfix_notation_destr(&notation);

    return result;
}



size_t max(size_t a, size_t b)
{
    return a >= b ? a : b;
}



////////////////////////////////////////
//// The bigint interface
////////////////////////////////////////

//! Consecutively cuts MAX_UINT32_LENGTH (or less) characters off input string
//  from its right end until it is not empty, converts the cut string into
//  uint32_t instance and pushes it back into newly created bigint instance.
//  Returns the created bigint pointer or NULL if error occurred.
bigint* bigint_from_string(const char *src_str, size_t str_length)
{
    if(!src_str)
    {
        return NULL;
    }

    bigint *new_bint = bigint_create_empty(1);
    if(!new_bint)
    {
        return NULL;
    }

    char buf[MAX_UINT32_LENGTH + 1];

    for( ; str_length > MAX_UINT32_LENGTH; str_length -= MAX_UINT32_LENGTH)
    {
        strncpy(buf, &(src_str[str_length - MAX_UINT32_LENGTH]), MAX_UINT32_LENGTH); // cuts another MAX_UINT32_LENGTH digits from the string
        buf[MAX_UINT32_LENGTH] = '\0';

        if(!bigint_pushback(new_bint, (uint32_t)strtoul(buf, NULL, 10)))
        {
            bigint_destr(&new_bint);
            return NULL;
        }
    }

    strncpy(buf, src_str, str_length); // writes remaining MAX_UINT32_LENGTH or less characters
    buf[str_length] = '\0';

    if(!bigint_pushback(new_bint, (uint32_t)strtoul(buf, NULL, 10)))
    {
        bigint_destr(&new_bint);
        return NULL;
    }

    bigint_remove_leading_zeros(new_bint);

    return new_bint;
}



//! Consecutively cuts MAX_UINT32_LENGTH (or less) digits from input int64_t
//  value until it becomes equal to 0, converts the cut value into uint32_t
//  instance and pushes it back into newly created bigint instance. Returns the
//  created bigint pointer or NULL if error occurred. Sets the result sign
//  accoringly.
bigint* bigint_from_int64(int64_t src_val)
{
    bigint *new_bint = bigint_create_empty(1);
    if(!new_bint)
    {
        return NULL;
    }

    if(src_val < 0)
    {
        src_val *= -1;
        new_bint->sign = NEGATIVE;
    }

    if(src_val == 0)
    {
        if(!bigint_pushback(new_bint, 0))
        {
            bigint_destr(&new_bint);
            return NULL;
        }

        return new_bint;
    }

    for( ; src_val > 0; src_val /= BASE_DENARY)
    {
        if(!bigint_pushback(new_bint, (uint32_t)(src_val % BASE_DENARY)))
        {
            bigint_destr(&new_bint);
            return NULL;
        }
    }

    return new_bint;
}



//! Frees the allocated memory for a bigint instance and nullifies the passed
//  pointer value.
void bigint_destr(bigint **origin)
{
    if(!origin || !(*origin))
    {
        return;
    }

    free((*origin)->value);
    free(*origin);
    *origin = NULL;
}



//! Converts the passed bigint instance into string. Returns the pointer to the
//  allocated string or NULL if error occurred.
//! Firstly takes the value of the highest significance uint32_t value of the
//  passed bigint as it might be of any length and prints it to the result
//  string. Then consecutively takes another uint32_t value as it MUST be of
//  MAX_UINT32_LENGTH and prints it to the result string.
char* bigint_to_string(const bigint *src_bint)
{
    if(!src_bint)
    {
        return NULL;
    }

    char *result = (char*)malloc((src_bint->length * MAX_UINT32_LENGTH + 1) * sizeof(char)); // allocates the string of the maximum possible length for the passed bigint;
    if(!result)
    {
        return NULL;
    }

    char *str_iter = result; // the result string iterator;
    size_t i = src_bint->length; // the passed bigint iterator. Goes from last to the first uint32_t value;

    snprintf(str_iter, MAX_UINT32_LENGTH + 1, "%u", src_bint->value[i - 1]); // print MAX_UINT32_LENGTH (or less) digits into the result string + '\0' char;
    str_iter += strlen(str_iter); // shifts the iterator right by the printed value length;

    for(--i; i > 0; --i) // repeat for the remaining uint32_t values;
    {
        // will ALWAYS print MAX_UINT32_LENGTH (if less substitudes the
        // remaining digits with 0) digits into the result string + '\0' char at
        // the end ('\0' gets rewritten by the next printed uint32_t).
        snprintf(str_iter, MAX_UINT32_LENGTH + 1, "%09u", src_bint->value[i - 1]);
        str_iter += strlen(str_iter);
    }

    return result;
}



//! Prints the passed bigint instance into stream.
bool bigint_print(FILE *stream, const bigint *src_bint)
{
    if(!stream || !src_bint)
    {
        return false;
    }

    if(src_bint->sign == NEGATIVE)
    {
        fputc('-', stream);
    }

    char *bint_str = bigint_to_string(src_bint);
    if(!bint_str)
    {
        return false;
    }

    fprintf(stream, "%s\n", bint_str);

    free(bint_str);

    return true;
}



//! Next static fuctions are only called inside the listed above set of bigint
//  functions where the passed arguments are guaranteed to be valid. Therefore
//  the passed arguments are considered to be valid and are not explicitly
//  checked there.

//! Creates the empty instance of bigint of the [cap] capacity.
static bigint* bigint_create_empty(size_t cap)
{
    if(cap == 0)
    {
        return NULL;
    }

    bigint *new_bint = (bigint*)malloc(sizeof(bigint));
    if(!new_bint)
    {
        return NULL;
    }

    new_bint->value = (uint32_t*)malloc(cap * sizeof(uint32_t));
    if(!new_bint->value)
    {
        free(new_bint);
        return NULL;
    }

    new_bint->sign = POSITIVE;
    new_bint->length = 0;
    new_bint->capacity = cap;

    return new_bint;
}



//! Pushes the passed uint32_t value to the end of the passed bigint instance.
//  The pushed value is considered to be the most significant one. Expands the
//  internal array if there are not enough space to add another value.
static bool bigint_pushback(bigint *dest_bint, uint32_t src_val)
{
    if(dest_bint->length >= dest_bint->capacity)
    {
        if(!bigint_expand(dest_bint, dest_bint->length + 1))
        {
            return false;
        }
    }

    dest_bint->value[dest_bint->length] = src_val;
    ++dest_bint->length;

    return true;
}



//! Pushes the passed uint32_t value to the beginning of the passed bigint
//  instance. The pushed value is considered to the the least significant one.
//  Expands the internal array if there are not enough space to add another
//  value.
static bool bigint_pushfront(bigint *dest_bint, uint32_t src_val)
{
    if(dest_bint->length >= dest_bint->capacity)
    {
        if(!bigint_expand(dest_bint, dest_bint->length + 1))
        {
            return false;
        }
    }

    for(size_t i = dest_bint->length; i > 0; --i) // shifts all values right by 1;
    {
        dest_bint->value[i] = dest_bint->value[i - 1];
    }

    dest_bint->value[0] = src_val; // writes the passed value to the vacant element;
    ++dest_bint->length;

    return true;
}



//! Expands the current capacity of the passed bigint instance with a new one if
//  it is bigger that previous one.
static bool bigint_expand(bigint *origin, size_t new_cap)
{
    if(new_cap <= origin->capacity)
    {
        return true;
    }

    uint32_t *new_value = (uint32_t*)realloc(origin->value, sizeof(uint32_t) * new_cap);
    if(!new_value)
    {
        return false;
    }
    origin->value = new_value;
    origin->capacity = new_cap;

    return true;
}



//! Decreases the length of the bigint until the most significant value is not
//  0. DOES NOT affect memory in any way.
static void bigint_remove_leading_zeros(bigint *origin)
{
    while(origin->value[origin->length - 1] == 0 && origin->length > 1)
    {
        --origin->length;
    }
}



//! Compares absolute values of two bigints. Returns 1, if bint_left is greater
//  than bint_right; -1 if bint_left is less than bint_right; 0 if they are
//  equal.
int bigint_compare(const bigint *left, const bigint *right)
{
    if(!left || !right)
    {
        return 0;
    }

    for(size_t i = max(left->length, right->length); i > 0; --i)
    {
        uint32_t left_uint = i > left->length ? 0 : left->value[i - 1];
        uint32_t right_uint = i > right->length ? 0 : right->value[i - 1];
        if(left_uint > right_uint)
        {
            return 1;
        }
        if(left_uint < right_uint)
        {
            return -1;
        }
    }

    return 0;
}



//! Next functions perform mathematical operations between two bigints and write
//  the result into a newly allocated instance of bigint which has to be freed
//  manually. Considering the combinations of the signs of passed bigints, these
//  call a corresponding function from the set below.

//! Performs an certain addition-corresponding (addition or subtraction)
//  operation considering the sign of both bigint values.
bigint* bigint_add(const bigint *left, const bigint *right)
{
    if(!left || !right)
    {
        return NULL;
    }

    bigint *result = bigint_create_empty(max(left->length, right->length) + 1); // alocates the capacity of maximum possible value;
    if(!result)
    {
        return NULL;
    }

    if(left->sign == NEGATIVE && right->sign == POSITIVE)
    {
        if(!bigint_subtract_values(right, left, result))
        {
            bigint_destr(&result);
            return NULL;
        }

        return result;
    }

    if(left->sign == POSITIVE && right->sign == NEGATIVE)
    {
        if(!bigint_subtract_values(left, right, result))
        {
            bigint_destr(&result);
            return NULL;
        }

        return result;
    }

    result->sign = left->sign;
    if(!bigint_add_values(left, right, result))
    {
        bigint_destr(&result);
        return NULL;
    }

    return result;
}



//! Performs an certain subtraction-corresponding (addition or subtraction)
//  operation considering the sign of both bigint values.
bigint* bigint_subtract(const bigint *left, const bigint *right)
{
    if(!left || !right)
    {
        return NULL;
    }

    bigint *result = bigint_create_empty(max(left->length, right->length) + 1);
    if(!result)
    {
        return NULL;
    }

    if(left->sign == NEGATIVE && right->sign == POSITIVE)
    {
        result->sign = NEGATIVE;
        if(!bigint_add_values(left, right, result))
        {
            bigint_destr(&result);
            return NULL;
        }

        return result;
    }

    if(left->sign == POSITIVE && right->sign == NEGATIVE)
    {
        result->sign = POSITIVE;
        if(!bigint_add_values(left, right, result))
        {
            bigint_destr(&result);
            return NULL;
        }

        return result;
    }

    if(left->sign == NEGATIVE && right->sign == NEGATIVE)
    {
        if(!bigint_subtract_values(right, left, result))
        {
            bigint_destr(&result);
            return NULL;
        }

        return result;
    }

    if(!bigint_subtract_values(left, right, result))
    {
        bigint_destr(&result);
        return NULL;
    }

    return result;
}



//! Performs a multiplication operation considering the sign of both bigint
//  values.
bigint* bigint_multiply(const bigint *left, const bigint *right)
{
    if(!left || !right)
    {
        return NULL;
    }

    bigint *result = bigint_create_empty(left->length + right->length);
    if(!result)
    {
        return NULL;
    }

    if(!bigint_multiply_values(left, right, result))
    {
        bigint_destr(&result);
        return NULL;
    }
    result->sign = left->sign * right->sign;

    return result;
}



//! Performs a division operation considering the sign of both bigint values.
bigint* bigint_divide(const bigint *left, const bigint *right)
{
    if(!left || !right)
    {
        return NULL;
    }

    bigint *bint_zero = bigint_from_int64(0);
    if(bigint_compare(right, bint_zero) == 0)
    {
        bigint_destr(&bint_zero);
        return NULL;
    }
    bigint_destr(&bint_zero);

    bigint *result = bigint_create_empty(left->length);
    if(!result)
    {
        return NULL;
    }

    if(!bigint_divide_values(left, right, result))
    {
        bigint_destr(&result);
        return NULL;
    }
    result->sign = left->sign * right->sign;

    return result;
}



//! Next functions perform mathematical operations between absolute values of
//  two bigints and write the result into existing binint instance. These are
//  called exclusevly by the functions from the list above, where passed
//  arguments are guaranteed.

//! Performs the long arithmetic addition of two bigint values.
static bool bigint_add_values(const bigint *left, const bigint *right, bigint *result)
{
    size_t i = 0;

    for(uint32_t carry = 0; i < max(left->length, right->length) || carry; ++i)
    {
        uint32_t left_uint = i < left->length ? left->value[i] : 0;
        uint32_t right_uint = i < right->length ? right->value[i] : 0;
        result->value[i] = left_uint + right_uint + carry;

        if(result->value[i] >= BASE_DENARY)
        {
            result->value[i] -= BASE_DENARY;
            carry = 1;
        }
        else
        {
            carry = 0;
        }
    }

    result->length = i;

    return true;
}



//! Performs the long arithmetic subtraction of two bigint values.
static bool bigint_subtract_values(const bigint *left, const bigint *right, bigint *result)
{
    if(bigint_compare(left, right) == -1)
    {
        const bigint *tmp = left;
        left = right;
        right = tmp;
        result->sign = NEGATIVE;
    }

    size_t i = 0;
    for(uint32_t carry = 0; i < left->length || carry; ++i)
    {
        int64_t left_uint = left->value[i];
        int64_t right_uint = i < right->length ? right->value[i] : 0;
        int64_t intermediate = left_uint - right_uint - carry;
        if(intermediate < 0)
        {
            carry = 1;
            result->value[i] = (uint32_t)(intermediate + BASE_DENARY);
            // safe cast since any element of bigint_t.value is less than
            // BASE_DENARY and bigger than 0 so their subtraction cannot exceed
            // the BASE_DENARY value;
        }
        else
        {
            carry = 0;
            result->value[i] = (uint32_t)intermediate;
            // safe cast since any element of bigint_t.value is less than
            // BASE_DENARY and their subtraction cannot exceed the max uint32_t
            // value;
        }
    }
    result->length = i;

    bigint_remove_leading_zeros(result);

    return true;
}



//! Performs the long arithmetic multiplication of two bigint values.
static bool bigint_multiply_values(const bigint *left, const bigint *right, bigint *result)
{
    result->length = 0;

    for(size_t i = 0, carry = 0; i < left->length; ++i)
    {
        for(size_t j = 0; j < right->length || carry; ++j)
        {
            if(i + j == result->length)
            {
                bigint_pushback(result, 0);
            }

            uint64_t left_uint = left->value[i];
            uint64_t right_uint = j < right->length ? right->value[j] : 0;
            uint64_t intermediate = left_uint * right_uint + carry;

            result->value[i + j] += (uint32_t)(intermediate % BASE_DENARY);
            carry = (uint32_t)(intermediate / BASE_DENARY);
        }
    }

    return true;
}



//! Performs the long arithmetic division of two bigint values.
//  OH GOD PLS NO. NO-O!
static bool bigint_divide_values(const bigint *left, const bigint *right, bigint *result)
{
    if(bigint_compare(left, right) == -1)
    {
        result->value[0] = 0;
        result->length = 1;
        return true;
    }

    bigint *bint_current = bigint_create_empty(left->length);
    if(!bint_current)
    {
        return false;
    }

    bigint *bint_multiplicator = bigint_create_empty(1);
    if(!bint_multiplicator)
    {
        bigint_destr(&bint_current);
        return false;
    }
    bint_multiplicator->length = 1;

    for(int i = (int)left->length - 1; i >= 0; --i)
    {
        bigint_pushfront(bint_current, left->value[i]);
        uint32_t max_dividor = 0;
        uint32_t left_end = 0;
        uint32_t right_end = BASE_DENARY;
        // Select the maximum number max_dividor, such is that right * max_dividor <= current_value
        while(left_end <= right_end)
        {
            uint32_t multiplicator = (left_end + right_end) >> 1;
            bint_multiplicator->value[0] = multiplicator;
            bigint *bint_mult_checker = bigint_multiply(right, bint_multiplicator);
            if(!bint_mult_checker)
            {
                bigint_destr(&bint_current);
                bigint_destr(&bint_multiplicator);
                return false;
            }
            if(bigint_compare(bint_mult_checker, bint_current) < 1)
            {
                max_dividor = multiplicator;
                left_end = multiplicator + 1;
            }
            else
            {
                right_end = multiplicator - 1;
            }
            bigint_destr(&bint_mult_checker);
        }

        result->value[i] = max_dividor;
        ++result->length;

        bint_multiplicator->value[0] = max_dividor;
        bigint *bint_mult_checker = bigint_multiply(right, bint_multiplicator);
        if(!bint_mult_checker)
        {
            bigint_destr(&bint_current);
            bigint_destr(&bint_multiplicator);
            return false;
        }

        bigint *to_destroy = bint_current;
        bint_current = bigint_subtract(bint_current, bint_mult_checker);
        bigint_destr(&to_destroy);
        bigint_destr(&bint_mult_checker);

        if(!bint_current)
        {
            bigint_destr(&bint_multiplicator);
            return false;
        }
    }

    bigint_destr(&bint_current);
    bigint_destr(&bint_multiplicator);

    bigint_remove_leading_zeros(result);

    return true;
}



////////////////////////////////////////
//// The token interface
////////////////////////////////////////

//! Gets a token from [src_str] shifted by [str_iter] characters. Shifts
//  str_iter forward by the length of the taken token.
token_t* token_get(const char *src_str, size_t *str_iter)
{
    if(!src_str || !str_iter)
    {
        return NULL;
    }

    token_t *new_token = (token_t*)malloc(sizeof(token_t));
    if(!new_token)
    {
        return NULL;
    }

    char literal = src_str[*str_iter];
    ++(*str_iter);

    while(literal == ' ') // ignore all spaces before
    {
        literal = src_str[*str_iter];
        ++(*str_iter);
    }

    if(token_get_operator(literal, new_token))
    {
        return new_token;
    }

    --(*str_iter);

    if(token_get_operand(src_str, str_iter, new_token))
    {
        return new_token;
    }

    token_destr(&new_token);
    return NULL;
}



//! Frees the allocated memory for a token instance and nullifies the passed
//  pointer value.
void token_destr(token_t **origin)
{
    if(!origin || !(*origin))
    {
        return;
    }

    if((*origin)->kind == NUMBER)
    {
        bigint_destr(&(*origin)->data.bigint);
    }

    free(*origin);
    *origin = NULL;
}



//! Tries to interpret a character as an operator.
static bool token_get_operator(char literal, token_t *token_ptr)
{
    switch(literal)
    {
    case '+':
        token_ptr->kind = ADDITION;
        token_ptr->data.operation_fn = &bigint_add;
        return true;

    case '-':
        token_ptr->kind = SUBTRACTION;
        token_ptr->data.operation_fn = &bigint_subtract;
        return true;

    case '*':
        token_ptr->kind = MULTIPLICATION;
        token_ptr->data.operation_fn = &bigint_multiply;
        return true;

    case '/':
        token_ptr->kind = DIVISION;
        token_ptr->data.operation_fn = &bigint_divide;
        return true;

    case '(':
        token_ptr->kind = LEFT_PARENTHESIS;
        token_ptr->data.operation_fn = NULL;
        return true;

    case ')':
        token_ptr->kind = RIGHT_PARENTHESIS;
        token_ptr->data.operation_fn = NULL;
        return true;

    case '\n':
    case '\0':
        token_ptr->kind = END_OF_EXPRETION;
        token_ptr->data.operation_fn = NULL;
        return true;

    case EOF:
        token_ptr->kind = END_OF_FILE;
        token_ptr->data.operation_fn = NULL;
        return true;

    default:
        token_ptr->kind = NONE;
        token_ptr->data.operation_fn = NULL;
        return false;
    }
}



//! Tries to get an operand from passed string.
static bool token_get_operand(const char *src_str, size_t *str_iter, token_t *token_ptr)
{
    size_t buf_size = BIGINT_STR_GROWTH + 1;
    char *buf = (char*)malloc(buf_size * sizeof(char));
    if(!buf)
    {
        return false;
    }

    char literal = src_str[*str_iter];
    ++(*str_iter);

    size_t i = 0;
    for( ; isdigit(literal); ++i)
    {
        if(i == buf_size)
        {
            buf_size += BIGINT_STR_GROWTH;

            char *new_buf = (char*)realloc(buf, buf_size * sizeof(char));
            if(!new_buf)
            {
                free(buf);
                return false;
            }
            buf = new_buf;
        }

        buf[i] = literal;
        literal = src_str[*str_iter];
        ++(*str_iter);
    }

    buf[i] = '\0';

    if(i == 0)
    {
        token_ptr->kind = NONE;
        free(buf);
        return false;
    }

    --(*str_iter);

    token_ptr->kind = NUMBER;
    token_ptr->data.bigint = bigint_from_string(buf, i);

    free(buf);

    return true;
}



////////////////////////////////////////
//// The token_stack interface
////////////////////////////////////////

//! Creates the empty instance of token stack of the [cap] capacity.
token_stack* token_stack_create(size_t cap)
{
    if(cap == 0)
    {
        return NULL;
    }

    token_stack *new_tstack = (token_stack*)malloc(sizeof(token_stack));
    if(!new_tstack)
    {
        return NULL;
    }

    new_tstack->array = (token_t*)malloc(cap * sizeof(token_t));
    if(!new_tstack->array)
    {
        free(new_tstack);
        return NULL;
    }

    new_tstack->capacity = cap;
    new_tstack->length = 0;

    return new_tstack;
}



//! Frees the allocated memory for a token stack instance and nullifies the
//  passed pointer value.
void token_stack_destr(token_stack **origin)
{
    if(!origin || !(*origin))
    {
        return;
    }

    do
    {
        token_t token = token_stack_pop(*origin);
        if(token.kind == NUMBER)
        {
            bigint_destr(&(token.data.bigint));
        }
    } while(!token_stack_is_empty(*origin));

    free((*origin)->array);
    free(*origin);
    *origin = NULL;
}



//! Pushes the passed value to the top of [dest] token stack by value.
bool token_stack_push(token_stack *dest, const token_t *value)
{
    if(!dest)
    {
        return false;
    }

    if(token_stack_is_full(dest))
    {
        if(!token_stack_expand(dest, dest->capacity + TOKEN_STACK_GROWTH))
        {
            return false;
        }
    }

    dest->array[dest->length] = *value;
    ++dest->length;

    return true;
}



//! Pops the top value of the [origin] token stack.
token_t token_stack_pop(token_stack *origin)
{
    if(!origin || token_stack_is_empty(origin))
    {
        token_t empty_token = { .kind = NONE, .data = {NULL} };
        return empty_token;
    }

    --origin->length;
    return origin->array[origin->length];
}



//! Returns the top value of passed token stack.
const token_t* token_stack_peek(const token_stack *origin)
{
    if(!origin || token_stack_is_empty(origin))
    {
        return NULL;
    }

    return &(origin->array[origin->length - 1]);
}



//! Expands the current capacity of the passed token stack instance with a new
//  one if it is bigger than previous capacity.
static bool token_stack_expand(token_stack *origin, size_t cap)
{
    if(cap <= origin->capacity)
    {
        return true;
    }

    token_t *new_data = (token_t*)realloc(origin->array, cap * sizeof(token_t));
    if(!new_data)
    {
        return false;
    }

    origin->array = new_data;
    origin->capacity = cap;

    return true;
}



//! Returns true if the passed token stack is empty. False otherwise.
static bool token_stack_is_empty(const token_stack *tstack)
{
    return (tstack->length == 0);
}



//! Returns true if the passed token stack is full. False otherwise.
static bool token_stack_is_full(const token_stack *tstack)
{
    return (tstack->length == tstack->capacity);
}




////////////////////////////////////////
//// The postfix_notation interface
////////////////////////////////////////

//! Creates an empty instance of postfix notation;
postfix_notation* postfix_notation_create_empty()
{
    postfix_notation *new_notation = (postfix_notation*)malloc(sizeof(postfix_notation));
    if(!new_notation)
    {
        return NULL;
    }

    new_notation->operands = token_stack_create(TOKEN_STACK_GROWTH);
    if(!new_notation->operands)
    {
        free(new_notation);
        return NULL;
    }

    new_notation->operators = token_stack_create(TOKEN_STACK_GROWTH);
    if(!new_notation->operators)
    {
        token_stack_destr(&new_notation->operands);
        free(new_notation);
        return NULL;
    }

    return new_notation;
}



//! Frees the allocated memory for a postfix notation instance and nullifies the
//  passed pointer value.
void postfix_notation_destr(postfix_notation **origin)
{
    if(!origin || !(*origin))
    {
        return;
    }

    token_stack_destr(&(*origin)->operands);
    token_stack_destr(&(*origin)->operators);
    free(*origin);
    *origin = NULL;
}



//! Adds another token into ints correspondig stack of the passed notation.
//  Reduces the notation if possible.
bool postfix_notation_add_token(postfix_notation *dest, token_t *token)
{
    if(!token)
    {
        return false;
    }

    // if the next passed operator is RIGHT_PARENTHESIS reduce the notation until
    // the neares LEFT_PARENTHESIS or until the operators stack is not empty.
    if(token->kind == RIGHT_PARENTHESIS)
    {
        while(!token_stack_is_empty(dest->operators) && token_stack_peek(dest->operators)->kind != LEFT_PARENTHESIS)
        {
            if(!postfix_notation_reduce(dest))
            {
                return false;
            }
        }

        token_stack_pop(dest->operators); // remove the found LEFT_PARENTHESIS.

        return true;
    }

    // reduce the notation until the previous operator is of a higher priority
    // than the new one.
    while(postfix_notation_can_reduce(dest, token))
    {
        if(!postfix_notation_reduce(dest))
        {
            return false;
        }
    }

    // push the new token to the corresponding stack.
    switch(token->kind)
    {
    case NONE:
    case END_OF_EXPRETION:
    case END_OF_FILE:
        break;

    case NUMBER:
        if(!token_stack_push(dest->operands, token))
        {
            return false;
        }
        break;

    default:
        if(!token_stack_push(dest->operators, token))
        {
            return false;
        }
        break;
    }

    return true;
}



//! Determines the priority value for the passed token.
//  The operator with bigger priority forces the postfix notation to be reduced
//  until the previous operator is of the lower priority in the operators stack.
//  - NUMBER is not an operator;
//  - LEFT_PARENTHESIS can only be reduced by RIGHT_PARENTHESIS explicitly;
//  - MULTIPLICATION and DIVISION can only reduce each other;
//  - ADDITION and SUBTRACTION can reduce each other, MULTIPLICATION and DIVISION;
//  - END_OF_EXPRETION and END_OF_FILE reduce any operator.
static int postfix_notation_prioritize(const token_t *token)
{
    switch(token->kind)
    {
    case NUMBER:
    case LEFT_PARENTHESIS:
    case RIGHT_PARENTHESIS:
        return -1;

    case MULTIPLICATION:
    case DIVISION:
        return 1;

    case ADDITION:
    case SUBTRACTION:
        return 2;

    case END_OF_EXPRETION:
    case END_OF_FILE:
        return 3;

    default:
        return 0;
    }
}



//! Determines whether the passed token can reduce the previous one.
static bool postfix_notation_can_reduce(const postfix_notation *origin, const token_t *next_token)
{
    if(token_stack_is_empty(origin->operators))
    {
        return false;
    }

    int priority_prev = postfix_notation_prioritize(token_stack_peek(origin->operators));
    int priority_new = postfix_notation_prioritize(next_token);

    return (priority_prev > 0 && priority_new > 0 && priority_new >= priority_prev);
}



//! Reduces the top-most operator in operators stack.
//  There is no reduction to be done (still valid) if the operators stack is empty.
//  All operators are handled as binary (unary operators have a fictive 0 as left operand).
//  The result operand is pushed at the top of the operands stack.
//  Thus any result expression MUST have as many math operators as operands - 1.
//  The last remaining operand is the result.
static bool postfix_notation_reduce(postfix_notation *origin)
{
    operation_fn_t action = token_stack_pop(origin->operators).data.operation_fn;
    if(!action)
    {
        return true;
    }

    bigint *right = token_stack_pop(origin->operands).data.bigint;
    bigint *left = token_stack_pop(origin->operands).data.bigint;
    if(!right || !left)
    {
        return false;
    }

    token_t result = { .kind = NUMBER, .data.bigint = action(left, right) };

    if(!result.data.bigint || !token_stack_push(origin->operands, &result))
    {
        bigint_destr(&left);
        bigint_destr(&right);

        return false;
    }

    bigint_destr(&left);
    bigint_destr(&right);

    return true;
}
