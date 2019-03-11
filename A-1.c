/*
Бабаян Владимир - АПО-12
Задача A-1. Задача о поиске максимума
Time limit:    14 s
Memory limit:  64 M

Составить программу поиска M наибольших чисел в заданном массиве целых (типа int) чисел A длины N. Числа M, N и элементы массива A подаются на стандартный ввод программы в следующем порядке:
N
A1 A2 ... AN
M

Процедура поиска должна быть оформлена в виде отдельной функции, которой подается на вход массив целых чисел, выделенный в динамической памяти, его длина и требуемое количество максимальных чисел. На выход функция должна возвращать динамически выделенный массив с найденными значениями.

Программа должна уметь обрабатывать ошибки во входных данных. В случае возникновения ошибки нужно вывести в поток стандартного вывода сообщение "[error]" и завершить выполнение программы.

ВАЖНО! Программа в любом случае должна возвращать 0. Не пишите return -1, exit(1) и т.п. Даже если обнаружилась какая-то ошибка, все равно необходимо вернуть 0! (и напечатать [error] в stdout).

Examples

Input
5
42 13 100 121 1
3

Output
121 100 42
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>


#define LOG_ERROR puts("[error]\n")



//! Reads [dest_size] number of integers from [stream] and writes to [dest] array.
//! Returns true is specified number of elements was successfully written. False otherwise.
bool get_int_array(FILE *stream, size_t dest_size, int *dest);
//! Finds maximum value of [src_size] elements of [src] array that is less than [limit].
//! Returns found value or 0 if passed [src] pointer is invalid.
int find_limited_max(const int *src, size_t src_size, int limit);
//! Finds [dest_count] number of maximum values of [src_size] elements of [src] array.
//! Returns address of newly allocated memory if passed [src] pointer is valid. NULL otherwise.
int* find_n_max(const int *src, size_t src_size, size_t dest_count);
//! Prints [src] int array into [stream].
//! Returns nothing.
void print_int_array(FILE *stream, const int *src, size_t src_size);



int main()
{
    size_t array_size = 0;
    if(scanf("%lu", &array_size) != 1)
    {
        LOG_ERROR;
        return 0;
    }

    int *array = (int*)malloc(array_size * sizeof(int));
    if(!array)
    {
        LOG_ERROR;
        return 0;
    }
    if(!get_int_array(stdin, array_size, array))
    {
        LOG_ERROR;
        free(array);
        return 0;
    }

    size_t max_count = 0;
    if((scanf("%lu", &max_count) != 1) || (max_count > array_size))
    {
        LOG_ERROR;
        free(array);
        return 0;
    }

    if(max_count == 0)
    {
        free(array);
        return 0;
    }

    int *res = find_n_max(array, array_size, max_count);
    if(!res)
    {
        LOG_ERROR;
        free(array);
        return 0;
    }

    print_int_array(stdout, res, max_count);

    free(array);
    free(res);

    return 0;
}



bool get_int_array(FILE *stream, size_t dest_size, int *dest)
{
    if(!stream || !dest)
    {
        return false;
    }

    for(size_t i = 0; i < dest_size; ++i)
    {
        if(feof(stream) || (fscanf(stream, "%d", &dest[i]) != 1))
        {
            return false;
        }
    }

    return true;
}



int find_limited_max(const int *src, size_t src_size, int limit)
{
    if(!src)
    {
        return 0;
    }

    int result = src[0];
    for(size_t i = 0; i < src_size; ++i)
    {
        if((result >= limit) || (src[i] > result && src[i] < limit))
        {
            result = src[i];
        }
    }

    return result;
}



int* find_n_max(const int *src, size_t src_size, size_t dest_count)
{
    if(!src || (dest_count == 0))
    {
        return NULL;
    }

    if(dest_count > src_size)
    {
        dest_count = src_size;
    }

    int *result = (int*)malloc(dest_count * sizeof(int));
    if(!result)
    {
        return NULL;
    }

    result[0] = find_limited_max(src, src_size, INT_MAX);

    for(size_t i = 1; i < dest_count; ++i)
    {
        result[i] = find_limited_max(src, src_size, result[i - 1]);
    }

    return result;
}



void print_int_array(FILE *stream, const int *src, size_t src_size)
{
    if(!stream || !src)
    {
        return;
    }

    for(size_t i = 0; i < src_size; ++i)
    {
        fprintf(stream, "%d ", src[i]);
    }
}
