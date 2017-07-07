#ifndef _REXPR_H_
#define _REXPR_H_

#include <stdio.h>
#include <stdlib.h>

#include "print.h"

/*
        Поиск подстроки по регулярному выражению.
        Поиск можно проводить как в одной строке, так и в нескольких отдельных.
        Символы:
        .       один любой символ
        *       повторение символа 0 или более раз
        +       повторение символа 1 или более раз
        ()      объединение правил
        []      один любой символ из набора
        |       логическое или "ИЛИ"
        TODO
        /       экранирование символов выше
*/

enum rexpr_object_type {
        /*Типы объектов*/
        rexpr_object_type_DOT,
        rexpr_object_type_STAR,
        rexpr_object_type_PLUS,
        rexpr_object_type_ROUND_BRACKETS_OPEN,
        rexpr_object_type_ROUND_BRACKETS_CLOSE,
        rexpr_object_type_SQUARE_BRACKETS_OPEN,
        rexpr_object_type_SQUARE_BRACKETS_CLOSE,
        rexpr_object_type_LINE,
        rexpr_object_type_SLASH,
        rexpr_object_type_STRING,
        
        /*Параметры*/
        rexpr_object_type_start_ch = rexpr_object_type_DOT,
        rexpr_object_type_end_ch = rexpr_object_type_SLASH,
        rexpr_object_type_escape_ch = rexpr_object_type_SLASH,
        rexpr_object_type_unknown_ch = rexpr_object_type_STRING
};

typedef struct rexpr_object_str rexpr_object_str;
struct rexpr_object_str {
        char * str;
        size_t len;
};

typedef struct rexpr_object rexpr_object;
struct rexpr_object {
        rexpr_object * next;
        rexpr_object * child;
        
        unsigned int type;
        rexpr_object_str * str;
};

size_t rexpr_find(char * str, size_t str_len, char * opt, size_t opt_len, size_t * end_find_ch);

#endif