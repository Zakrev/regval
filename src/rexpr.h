#ifndef _REXPR_H_
#define _REXPR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
        Поиск подстроки по регулярному выражению.
        Поиск можно проводить как в одной строке, так и в нескольких отдельных.
        Символы:
        .       один любой символ
        *       повторение 0 или более раз
        +       повторение 1 или более раз
        ()      объединение правил
        []      один любой символ из набора, всё интерпритируется как символы, необходимо экранировать только закрывающюю скобку '[['
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

enum rexpr_object_type_second {
        /*Типы дополнительных объектов*/
        rexpr_object_type_second_ESCAPE,
        rexpr_object_type_second_NOT,
        rexpr_object_type_second_OR,
        
        /*Параметры*/
        rexpr_object_type_second_start_ch = rexpr_object_type_second_ESCAPE,
        rexpr_object_type_second_end_ch = rexpr_object_type_second_OR,
        rexpr_object_type_second_unknown_ch
};

enum rexpr_escape_type {
        /*Различные escape-символы для комад вида '\n', используется только для SQUARE_BRACKETS '[a-z \n\t]'*/
        rexpr_escape_type_NEWLINE,
        rexpr_escape_type_TAB,
        
        /*Параметры*/
        rexpr_escape_type_start_ch = rexpr_escape_type_NEWLINE,
        rexpr_escape_type_end_ch = rexpr_escape_type_TAB,
        rexpr_escape_type_unknown_ch
};

struct rexpr_object_str {
        char * str;
        ssize_t len;
};

#define MAX_CH_LEN 2

typedef struct rexpr_object_ch_range rexpr_object_ch_range;
struct rexpr_object_ch_range {
        struct rexpr_object_ch_range * next;
        
        char l[MAX_CH_LEN];
        char r[MAX_CH_LEN];
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

/*
        Многостроковый поиск
        
        Функция выдает новую строку, в которой продолжиться поиск
        void (* get_next_str)(char * str, ssize_t * start, ssize_t * end, void * data)
        str     указатель на новую строку
        start   начало данных в новой строке
        end     конец данных в новой строке
        data    указатель на данные(пользователя), которые нужны для работы функции
        
        Функцию определяет сам пользователь и передает в качестве аргумента
*/

/*
        Функция парсит регулярное выражение, создает его представление в структурах
        В случае ошибки в переменной end будет передан номер символа на котором возникла ошибка,
        либо -1 в случае неожиданного конца строки с выражением
*/
int parse_rexpr_object(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end);


/*
        Функция проверяет совпадение строки с регулярным выражением
        Если совпадения нет, возвращается rexpr_check_status_UNSUCCESS
        Если есть совпадение, то в end записывается последний символ совпавшей подстроки и возвращается rexpr_check_status_SUCCESS
        Если при проверке закончилась строка, то возвращается rexpr_check_status_END_OF_LINE
        В случае неудачи, *end останется в прежнем значении
        Функция НЕ ИЩЕТ подстроку в строке, а только проверяет совпадение, начиная с первого символа
*/
int check_str_rexpr_object(rexpr_object * parent, const char * str, ssize_t start, ssize_t * end, 
                        void (* get_next_str)(char *, ssize_t *, ssize_t *, void *),
                        void * data);


/*Освобождает память из под структур*/
void free_rexpr_objects(rexpr_object * parent);


/*
        Пример использования
        str     - строка
        opt     - регулярное выражение
        *end_substr   - последний символ найденой подстроки
        Функция возвращает позицию первого символа найденой подстроки
        Либо -1
*/
ssize_t rexpr_find(const char * str, ssize_t str_len, const char * opt, ssize_t opt_len, ssize_t * end_find_ch);

#endif