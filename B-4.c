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



typedef bigint* (*operation_fn)(const bigint*, const bigint*);



typedef union
{
    bigint *bigint;
    operation_fn operation_fn;
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
    size_t capacity;
    int top;
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
bigint* calculate(const char *src_str);
size_t max(size_t a, size_t b);



////////////////////////////////////////
//// The bigint interface
////////////////////////////////////////
bigint* bigint_from_string(const char *src_str);
bigint* bigint_from_int64(int64_t src_val);
void bigint_destr(bigint **origin);
void bigint_to_string(const bigint *src_bint, char **dest_str);
void bigint_print(FILE *stream, const bigint *src_bint);
bool bigint_shrink(bigint *origin);

static bigint* bigint_create_empty(size_t cap);
static bool bigint_pushback(bigint *dest_bint, uint32_t src_val);
static bool bigint_pushfront(bigint *dest_bint, uint32_t src_val);
static bool bigint_expand(bigint *origin, size_t new_cap);
static void bigint_remove_leading_zeros(bigint *origin);

int bigint_compare(const bigint *bint_a, const bigint *bint_b);
bigint* bigint_add(const bigint *bint_a, const bigint *bint_b);
bigint* bigint_subtract(const bigint *bint_a, const bigint *bint_b);
bigint* bigint_multiply(const bigint *bint_a, const bigint *bint_b);
bigint* bigint_divide(const bigint *bint_a, const bigint *bint_b);
bigint* bigint_negate(const bigint *bint_a);

static bool bigint_add_values(const bigint *bint_a, const bigint *bint_b, bigint *result);
static bool bigint_subtract_values(const bigint *bint_a, const bigint *bint_b, bigint *result);
static bool bigint_multiply_values(const bigint *bint_a, const bigint *bint_b, bigint *result);
static bool bigint_divide_values(const bigint *bint_a, const bigint *bint_b, bigint *result);



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
token_stack* token_stack_create(const token_t *src_array, size_t cap);
void token_stack_destr(token_stack **origin);
void token_stack_push(token_stack *dest, const token_t *value);
token_t token_stack_pop(token_stack *origin);
const token_t* token_stack_peek(const token_stack *origin);
bool token_stack_expand(token_stack *origin, size_t cap);
bool token_stack_is_empty(const token_stack *src);
bool token_stack_is_full(const token_stack *src);



////////////////////////////////////////
//// The postfix_notation interface
////////////////////////////////////////
postfix_notation* postfix_notation_create_empty();
void postfix_notation_destr(postfix_notation **origin);
bool postfix_notation_add_token(postfix_notation *dest, token_t *token);

static int postfix_notation_prioritize(const token_t *token);
static bool postfix_notation_can_reduce(const postfix_notation *origin, const token_t *next_operator);
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

        bigint *result = calculate(input_str);
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
bigint* calculate(const char *src_str)
{
    if(!src_str)
    {
        return NULL;
    }

    size_t str_iter = 0;
    token_kind prev = LEFT_PARENTHESIS;
    postfix_notation *notation = postfix_notation_create_empty();

    while(str_iter <= strlen(src_str))
    {
        token_t *token = token_get(src_str, &str_iter);
        if(!token)
        {
            postfix_notation_destr(&notation);
            return NULL;
        }

        // Unary minus
        if(prev == LEFT_PARENTHESIS && token->kind == SUBTRACTION)
        {
            token_t zero_token = {NUMBER, {NULL}};
            zero_token.data.bigint = bigint_from_int64(0);
            token_stack_push(notation->operands, &zero_token);
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

    bigint *result = token_stack_pop(notation->operands).data.bigint;

    if(!result || !token_stack_is_empty(notation->operators))
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
bigint* bigint_from_string(const char *src_str)
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

    for(int i = (int)strlen(src_str); i > 0; i -= MAX_UINT32_LENGTH)
    {
        char buf[MAX_UINT32_LENGTH + 1];

        if(i < MAX_UINT32_LENGTH)
        {
            strncpy(buf, src_str, (size_t)i);
            buf[i] = '\0';
        }
        else
        {
            strncpy(buf, &(src_str[i - MAX_UINT32_LENGTH]), MAX_UINT32_LENGTH);
            buf[MAX_UINT32_LENGTH] = '\0';
        }

        if(!bigint_pushback(new_bint, (uint32_t)strtoul(buf, NULL, 10)))
        {
            bigint_destr(&new_bint);
            return NULL;
        }
    }

    bigint_remove_leading_zeros(new_bint);

    return new_bint;
}



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



void bigint_destr(bigint **origin)
{
    free((*origin)->value);
    free(*origin);
    *origin = NULL;
}



void bigint_to_string(const bigint *src_bint, char **dest_str)
{
    if(!src_bint || !src_bint->value)
    {
        return;
    }

    *dest_str = (char*)malloc((src_bint->length * MAX_UINT32_LENGTH + 1) * sizeof(char));
    if(!(*dest_str))
    {
        return;
    }

    char *buf_ptr = (*dest_str);
    for(int i = (int)src_bint->length - 1; i >= 0; --i)
    {
        snprintf(buf_ptr, MAX_UINT32_LENGTH + 1, "%u", src_bint->value[i]);
        buf_ptr += strlen(buf_ptr);
    }
}



void bigint_print(FILE *stream, const bigint *src_bint)
{
    if(!stream || !src_bint || !src_bint->value)
    {
        return;
    }

    if(src_bint->sign == NEGATIVE)
    {
        fputc('-', stream);
    }

    char *bint_str = NULL;
    bigint_to_string(src_bint, &bint_str);
    if(!bint_str)
    {
        return;
    }

    fprintf(stream, "%s\n", bint_str);

    free(bint_str);
}



bool bigint_shrink(bigint *bint_ptr)
{
    if(!bint_ptr)
    {
        return false;
    }

    uint32_t *ptr = (uint32_t*)realloc(bint_ptr->value, sizeof(uint32_t) * bint_ptr->length);
    if(!ptr)
    {
        return false;
    }
    bint_ptr->value = ptr;
    bint_ptr->capacity = bint_ptr->length;

    return true;
}



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

    new_bint->value = (uint32_t*)malloc(sizeof(uint32_t) * cap);
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



static bool bigint_pushback(bigint *dest_bint, uint32_t src_val)
{
    if(!dest_bint)
    {
        return false;
    }

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



static bool bigint_pushfront(bigint *dest_bint, uint32_t src_val)
{
    if(!dest_bint)
    {
        return false;
    }

    if(dest_bint->length >= dest_bint->capacity)
    {
        if(!bigint_expand(dest_bint, dest_bint->length + 1))
        {
            return false;
        }
    }

    for(int i = (int)dest_bint->length - 1; i >= 0; --i)
    {
        dest_bint->value[i + 1] = dest_bint->value[i];
    }

    dest_bint->value[0] = src_val;
    ++dest_bint->length;

    return true;
}



static bool bigint_expand(bigint *origin, size_t new_cap)
{
    if(new_cap <= origin->capacity)
    {
        return true;
    }

    uint32_t *ptr = (uint32_t*)realloc(origin->value, sizeof(uint32_t) * new_cap);
    if(!ptr)
    {
        return false;
    }
    origin->value = ptr;
    origin->capacity = new_cap;

    return true;
}



static void bigint_remove_leading_zeros(bigint *origin)
{
    if(!origin || !origin->value)
    {
        return;
    }

    while(origin->value[origin->length - 1] == 0 && origin->length > 1)
    {
        --origin->length;
    }
}



int bigint_compare(const bigint *bint_a, const bigint *bint_b)
{
    if(!bint_a || !bint_b)
    {
        return 0;
    }

    for(size_t i = max(bint_a->length, bint_b->length); i > 0; --i)
    {
        uint32_t left = i > bint_a->length ? 0 : bint_a->value[i - 1];
        uint32_t right = i > bint_b->length ? 0 : bint_b->value[i - 1];
        if(left > right)
        {
            return 1;
        }
        if(left < right)
        {
            return -1;
        }
    }

    return 0;
}



bigint* bigint_add(const bigint *bint_a, const bigint *bint_b)
{
    if(!bint_a || !bint_a->value || !bint_b || !bint_b->value)
    {
        return NULL;
    }

    bigint *result = bigint_create_empty(max(bint_a->length, bint_b->length) + 1);
    if(!result)
    {
        return NULL;
    }

    if(bint_a->sign == NEGATIVE && bint_b->sign == POSITIVE)
    {
        if(!bigint_subtract_values(bint_b, bint_a, result))
        {
            bigint_destr(&result);
            return NULL;
        }

        return result;
    }

    if(bint_a->sign == POSITIVE && bint_b->sign == NEGATIVE)
    {
        if(!bigint_subtract_values(bint_a, bint_b, result))
        {
            bigint_destr(&result);
            return NULL;
        }

        return result;
    }

    result->sign = bint_a->sign;
    if(!bigint_add_values(bint_a, bint_b, result))
    {
        bigint_destr(&result);
        return NULL;
    }

    return result;
}



bigint* bigint_subtract(const bigint *bint_a, const bigint *bint_b)
{
    if(!bint_a || !bint_a->value || !bint_b || !bint_b->value)
    {
        return NULL;
    }

    bigint *result = bigint_create_empty(max(bint_a->length, bint_b->length) + 1);
    if(!result)
    {
        return NULL;
    }

    if(bint_a->sign == NEGATIVE && bint_b->sign == POSITIVE)
    {
        result->sign = NEGATIVE;
        if(!bigint_add_values(bint_a, bint_b, result))
        {
            bigint_destr(&result);
            return NULL;
        }

        return result;
    }

    if(bint_a->sign == POSITIVE && bint_b->sign == NEGATIVE)
    {
        result->sign = POSITIVE;
        if(!bigint_add_values(bint_a, bint_b, result))
        {
            bigint_destr(&result);
            return NULL;
        }

        return result;
    }

    if(bint_a->sign == NEGATIVE && bint_b->sign == NEGATIVE)
    {
        if(!bigint_subtract_values(bint_b, bint_a, result))
        {
            bigint_destr(&result);
            return NULL;
        }

        return result;
    }

    if(!bigint_subtract_values(bint_a, bint_b, result))
    {
        bigint_destr(&result);
        return NULL;
    }

    return result;
}



bigint* bigint_multiply(const bigint *bint_a, const bigint *bint_b)
{
    if(!bint_a || !bint_a->value || !bint_b || !bint_b->value)
    {
        return NULL;
    }


    bigint *result = bigint_create_empty(bint_a->length + bint_b->length);
    if(!result)
    {
        return NULL;
    }

    if(!bigint_multiply_values(bint_a, bint_b, result))
    {
        bigint_destr(&result);
        return NULL;
    }
    result->sign = bint_a->sign * bint_b->sign;

    return result;
}



bigint* bigint_divide(const bigint *bint_a, const bigint *bint_b)
{
    if(!bint_a || !bint_a->value || !bint_b || !bint_b->value)
    {
        return NULL;
    }

    bigint *result = bigint_create_empty(bint_a->length);
    if(!result)
    {
        return NULL;
    }

    if(!bigint_divide_values(bint_a, bint_b, result))
    {
        bigint_destr(&result);
        return NULL;
    }
    result->sign = bint_a->sign * bint_b->sign;

    return result;
}



bigint* bigint_negate(const bigint *bint_a)
{
    if(!bint_a || !bint_a->value)
    {
        return NULL;
    }

    bigint *result = bigint_create_empty(bint_a->length);
    if(!result)
    {
        return NULL;
    }

    result->sign = bint_a->sign * NEGATIVE;
    result->length = bint_a->length;
    result->capacity = bint_a->length;

    for(size_t i = 0; i < result->length; ++i)
    {
        result->value[i] = bint_a->value[i];
    }

    return result;
}



static bool bigint_add_values(const bigint *bint_a, const bigint *bint_b, bigint *result)
{
    if(!bint_a || !bint_a->value || !bint_b || !bint_b->value || !result || !result->value)
    {
        return false;
    }

    size_t i = 0;
    for(uint32_t carry = 0; i < max(bint_a->length, bint_b->length) || carry; ++i)
    {
        uint32_t left = i < bint_a->length ? bint_a->value[i] : 0;
        uint32_t right = i < bint_b->length ? bint_b->value[i] : 0;
        result->value[i] = left + right + carry;

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



static bool bigint_subtract_values(const bigint *bint_a, const bigint *bint_b, bigint *result)
{
    if(!bint_a || !bint_a->value || !bint_b || !bint_b->value || !result || !result->value)
    {
        return false;
    }

    if(bigint_compare(bint_a, bint_b) == -1)
    {
        const bigint *tmp = bint_a;
        bint_a = bint_b;
        bint_b = tmp;
        result->sign = NEGATIVE;
    }

    size_t i = 0;
    for(uint32_t carry = 0; i < bint_a->length || carry; ++i)
    {
        int64_t left = bint_a->value[i];
        int64_t right = i < bint_b->length ? bint_b->value[i] : 0;
        int64_t intermediate = left - right - carry;
        if(intermediate < 0)
        {
            carry = 1;
            result->value[i] = (uint32_t)(intermediate + BASE_DENARY);
        }
        else
        {
            carry = 0;
            result->value[i] = (uint32_t)intermediate;
        }
    }
    result->length = i;
    bigint_remove_leading_zeros(result);

    return true;
}



static bool bigint_multiply_values(const bigint *bint_a, const bigint *bint_b, bigint *result)
{
    if(!bint_a || !bint_a->value || !bint_b || !bint_b->value || !result || !result->value)
    {
        return false;
    }

    result->length = 0;

    for(size_t i = 0, carry = 0; i < bint_a->length; ++i)
    {
        for(size_t j = 0; j < bint_b->length || carry; ++j)
        {
            if(i + j == result->length)
            {
                bigint_pushback(result, 0);
            }

            uint64_t left = bint_a->value[i];
            uint64_t right = j < bint_b->length ? bint_b->value[j] : 0;
            uint64_t intermediate = left * right + carry;

            result->value[i + j] += (uint32_t)(intermediate % BASE_DENARY);
            carry = (uint32_t)(intermediate / BASE_DENARY);
        }
    }

    return true;
}



static bool bigint_divide_values(const bigint *bint_a, const bigint *bint_b, bigint *result)
{
    if(!bint_a || !bint_a->value || !bint_b || !bint_b->value || !result || !result->value)
    {
        return false;
    }

    if(bigint_compare(bint_a, bint_b) == -1)
    {
        result->value[0] = 0;
        result->length = 1;
        return true;
    }

    bigint *bint_current = bigint_create_empty(bint_a->length);
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

    for(int i = (int)bint_a->length - 1; i >= 0; --i)
    {
        bigint_pushfront(bint_current, bint_a->value[i]);
        uint32_t max_dividor = 0;
        uint32_t left_end = 0;
        uint32_t right_end = BASE_DENARY;
        // Select the maximum number max_dividor, such is that bint_b * max_dividor <= current_value
        while(left_end <= right_end)
        {
            uint32_t multiplicator = (left_end + right_end) >> 1;
            bint_multiplicator->value[0] = multiplicator;
            bigint *bint_mult_checker = bigint_multiply(bint_b, bint_multiplicator);
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
        bigint *bint_mult_checker = bigint_multiply(bint_b, bint_multiplicator);
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

    return true;
}



////////////////////////////////////////
//// The token interface
////////////////////////////////////////
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
    while(literal == ' ')
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



void token_destr(token_t **origin)
{
    if((*origin)->kind == NUMBER)
    {
        bigint_destr(&(*origin)->data.bigint);
    }

    free(*origin);
    *origin = NULL;
}



static bool token_get_operator(char literal, token_t *token_ptr)
{
    if(!token_ptr)
    {
        return false;
    }

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



static bool token_get_operand(const char *src_str, size_t *str_iter, token_t *token_ptr)
{
    if(!src_str || !str_iter || !token_ptr)
    {
        return false;
    }

    size_t buf_size = BIGINT_STR_GROWTH + 1;
    char *buf = (char*)malloc(buf_size * sizeof(char));
    if(!buf)
    {
        return false;
    }

    size_t i = 0;
    char literal = src_str[*str_iter];
    ++(*str_iter);
    for( ; isdigit(literal); ++i)
    {
        if(i == buf_size)
        {
            buf_size += BIGINT_STR_GROWTH;

            char *ptr = (char*)realloc(buf, buf_size * sizeof(char));
            if(!ptr)
            {
                free(buf);
                return false;
            }
            buf = ptr;
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
    token_ptr->data.bigint = bigint_from_string(buf);

    free(buf);
    return true;
}



////////////////////////////////////////
//// The token_stack interface
////////////////////////////////////////
token_stack* token_stack_create(const token_t *src_array, size_t cap)
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
    new_tstack->top = -1;

    if(!src_array)
    {
        return new_tstack;
    }

    for(size_t i = 0; i < cap; ++i)
    {
        token_stack_push(new_tstack, (src_array + i));
    }

    return new_tstack;
}



void token_stack_destr(token_stack **origin)
{
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



void token_stack_push(token_stack *dest, const token_t *value)
{
    if(!dest)
    {
        return;
    }

    if(token_stack_is_full(dest))
    {
        if(!token_stack_expand(dest, dest->capacity + TOKEN_STACK_GROWTH))
        {
            return;
        }
    }

    ++dest->top;
    dest->array[dest->top] = *value;
}



token_t token_stack_pop(token_stack *origin)
{
    if(!origin || token_stack_is_empty(origin))
    {
        token_t empty_token = {NONE, {NULL}};
        return empty_token;
    }

    return origin->array[origin->top--];
}



const token_t* token_stack_peek(const token_stack *origin)
{
    if(!origin || token_stack_is_empty(origin))
    {
        return NULL;
    }

    return &(origin->array[origin->top]);
}



bool token_stack_expand(token_stack *origin, size_t cap)
{
    if(!origin)
    {
        return false;
    }

    if(cap <= origin->capacity)
    {
        return true;
    }

    token_t *ptr = (token_t*)realloc(origin->array, cap * sizeof(token_t));
    if(!ptr)
    {
        return false;
    }
    origin->array = ptr;
    origin->capacity = cap;

    return true;
}



bool token_stack_is_empty(const token_stack *src)
{
    return (src->top == -1);
}



bool token_stack_is_full(const token_stack *src)
{
    return (src->top == (int)(src->capacity) - 1);
}




////////////////////////////////////////
//// The postfix_notation interface
////////////////////////////////////////
postfix_notation* postfix_notation_create_empty()
{
    postfix_notation *new_notation = (postfix_notation*)malloc(sizeof(postfix_notation));
    if(!new_notation)
    {
        return NULL;
    }

    new_notation->operands = token_stack_create(NULL, TOKEN_STACK_GROWTH);
    if(!new_notation->operands)
    {
        free(new_notation);
        return NULL;
    }

    new_notation->operators = token_stack_create(NULL, TOKEN_STACK_GROWTH);
    if(!new_notation->operators)
    {
        token_stack_destr(&new_notation->operands);
        free(new_notation);
        return NULL;
    }

    return new_notation;
}



void postfix_notation_destr(postfix_notation **origin)
{
    token_stack_destr(&(*origin)->operands);
    token_stack_destr(&(*origin)->operators);
    free(*origin);
    *origin = NULL;
}




bool postfix_notation_add_token(postfix_notation *dest, token_t *token)
{
    if(!token)
    {
        return false;
    }

    if(token->kind == RIGHT_PARENTHESIS)
    {
        while(!token_stack_is_empty(dest->operators) && token_stack_peek(dest->operators)->kind != LEFT_PARENTHESIS)
        {
            if(!postfix_notation_reduce(dest))
            {
                return false;
            }
        }
        token_stack_pop(dest->operators);

        return true;
    }

    while(postfix_notation_can_reduce(dest, token))
    {
        if(!postfix_notation_reduce(dest))
        {
            return false;
        }
    }

    switch(token->kind)
    {
        case NONE:
        case END_OF_EXPRETION:
        case END_OF_FILE:
            break;
        case NUMBER:
            token_stack_push(dest->operands, token);
            break;
        default:
            token_stack_push(dest->operators, token);
            break;
    }

    return true;
}



static int postfix_notation_prioritize(const token_t *token)
{
    if(!token)
    {
        return 0;
    }
// The operator with bigger priority pushes previous operator with lower priority out of operators stack.
    switch(token->kind)
    {
        case LEFT_PARENTHESIS:
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



static bool postfix_notation_can_reduce(const postfix_notation *origin, const token_t *next_token)
{
    if(token_stack_is_empty(origin->operators))
    {
        return false;
    }

    int priority_curr = postfix_notation_prioritize(token_stack_peek(origin->operators));
    int priority_new = postfix_notation_prioritize(next_token);

    return (priority_curr >= 0 && priority_new >= 0 && priority_new >= priority_curr);
}



static bool postfix_notation_reduce(postfix_notation *origin)
{
    operation_fn action = token_stack_pop(origin->operators).data.operation_fn;
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

    token_t result;
    result.kind = NUMBER;
    result.data.bigint = action(left, right);

    token_stack_push(origin->operands, &result);

    bigint_destr(&left);
    bigint_destr(&right);

    return true;
}
