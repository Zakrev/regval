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
        TODO
*/

enum rexpr_check_status {
        rexpr_check_status_SUCCESS,
        rexpr_check_status_UNSUCCESS,
        rexpr_check_status_END_OF_LINE
};

enum rexpr_object_type {
        /*Типы объектов*/
        rexpr_object_type_DOT,
        rexpr_object_type_STAR,
        rexpr_object_type_PLUS,
        rexpr_object_type_ROUND_BRACKETS_OPEN,
        rexpr_object_type_ROUND_BRACKETS_CLOSE,
        rexpr_object_type_SQUARE_BRACKETS_OPEN,
        rexpr_object_type_SQUARE_BRACKETS_CLOSE,
        rexpr_object_type_STRING,
        rexpr_object_type_CH_RANGE,
        
        /*Параметры*/
        rexpr_object_type_start_main,
        rexpr_object_type_start_ch = rexpr_object_type_DOT,
        rexpr_object_type_end_ch = rexpr_object_type_SQUARE_BRACKETS_CLOSE,
        rexpr_object_type_unknown_ch = rexpr_object_type_STRING
};

#define rexpr_escape_init '\\'
enum rexpr_escape_type {
        /*Различные escape-символы для комад вида '\n'*/
        rexpr_escape_type_NEWLINE,
        rexpr_escape_type_TAB,
        
        /*Параметры*/
        rexpr_escape_type_start_ch = rexpr_escape_type_NEWLINE,
        rexpr_escape_type_end_ch = rexpr_escape_type_TAB,
        rexpr_escape_type_unknown_ch
};

struct rexpr_object_str {
        char * str;
        size_t len;
};

typedef struct rexpr_object_ch_range rexpr_object_ch_range;
struct rexpr_object_ch_range {
        struct rexpr_object_ch_range * next;
        
        char l;
        char r;
};

typedef struct rexpr_object rexpr_object;
struct rexpr_object {
        rexpr_object * next;
        rexpr_object * child;
        
        unsigned int type;
        union {
                unsigned int type;
                struct rexpr_object_str str;
                struct rexpr_object_ch_range * ch_range;
        } data;
};

enum rexpr_object_answer_type {
        rexpr_object_answer_type_BREAK,
        rexpr_object_answer_type_CONTINUE
};

ssize_t rexpr_find(const char * str, ssize_t str_len, const char * opt, ssize_t opt_len, ssize_t * end_find_ch);

#endif