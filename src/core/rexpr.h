#ifndef _REXPR_H_
#define _REXPR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prefix_tree.h"

/*
	Тип, в котором будут храниться данные связанные с длинной байт
	Например позиция байта, длинна строки и т.д.
*/
typedef ssize_t bytes_t;

/*
	Тип, в котором хранится символ
*/
typedef unsigned char uchar_t;

#include "types.h"

enum rexpr_check_status {
	rexpr_check_status_SUCCESS,
	rexpr_check_status_UNSUCCESS,
	rexpr_check_status_END_OF_DATA
};

typedef struct rexpr_object_str rexpr_object_str;
struct rexpr_object_str {
	uchar_t * str;
	bytes_t len;
};

#define MAX_CH_LEN 4

typedef struct rexpr_object_ch_range rexpr_object_ch_range;
struct rexpr_object_ch_range {
	/*
		Структура хранит диапазон символов
	*/
	struct rexpr_object_ch_range * next;

	unsigned char bytes;
	uchar_t l[MAX_CH_LEN];
	uchar_t r[MAX_CH_LEN];
};

typedef struct rexpr_object_len_range rexpr_object_len_range;
struct rexpr_object_len_range {
	/*
		Структура хранит диапазон повторений, для {l,r}
	*/
	unsigned short l;
	unsigned short r;
};

typedef struct rexpr_object rexpr_object;
struct rexpr_object {
	/*
		Структура описывает основные объекты
	*/
	rexpr_object * next;
	rexpr_object * child;
	
	uchar_t type;
	uchar_t s_type;

	union {
		rexpr_object_str str;
		rexpr_object_len_range len_range;
		rexpr_object_ch_range * ch_range;
		unsigned int group_id;			//порядковый номер (от 1 до 4294967295) группы в паттерне (с права на лево)
										//для rexpr_object_type_start_main, этот параметр хранит количество именованных групп
	} data;
};

typedef struct rexpr_object_data rexpr_object_data;
struct rexpr_object_data {
	/*
		Структура описывает скомпилированное рег. выражение
	*/
	uchar_t * str;				//выражение
	bytes_t start;				//начало строки
	bytes_t end;				//конец строки

	ptree_node * groups;			//именованные группы
	unsigned int group_count;		//количество созданных груп
	uchar_t * brackets_map;			//порядок скобок в выражении
	unsigned int brackets_map_end;
};

typedef struct rexpr_object_result_group rexpr_object_result_group;
struct rexpr_object_result_group {
	/*
		Структура описывает результаты для группы
	*/
	rexpr_object_result_group * next;
	//начало строки
	unsigned int line_start;
	bytes_t start;
	
	//конец строки
	unsigned int line_end;
	bytes_t end;
};

typedef struct rexpr_object_result rexpr_object_result;
struct rexpr_object_result {
	/*
		Структура описывает параметры и результаты сравнения строки с регулярным выражением
	*/
	uchar_t * str;					//выражение
	bytes_t start;					//начало строки
	bytes_t end;					//конец строки

	rexpr_object_result_group ** group_result;	//массив результатов групп, индекс массива - порядковый номер группы (от 1 до 4294967295) в паттерне (с лева на право)
	unsigned int group_result_size;
	unsigned int line;							//количество переходов на новую строку, используется для позиционирования результатов групп

	/*
		Многострочный поиск
		get_next_str - указатель на функцию, определенную пользователем. Функция должна возвращать:
			str	указатель на новые данные, если данных нет, то должна быть равна переданному значению (т.е. меняться не должна)
			start	начало данных
			end		конец данных (НЕ ДЛИННА)
			line - id новой строки (id == 0 - первая строка)
		get_str_data - вспомогательная структура, может быть NULL

		Функция должна уметь возвращать следующую строку относительно параметра line (в функцию передается id текущей строки)
	*/
	void (* get_next_str)(char ** str, ssize_t * start, ssize_t * end, unsigned int * line, void * get_next_str_data);
	void * get_str_data;
};

/*
				Функции компилирования
*/

/*
	Функция компилирует регулярное выражение
	Возвращает
		позицию последнего символа	OK
		позицию ошибки в строке		ERR1
		-1				ERR2
*/
long long compile_rexpr(rexpr_object * root, uchar_t * expr, bytes_t expr_len);

/*
	Функция освобождает память из под скомпилированного выражения
*/
void free_rexpr(rexpr_object * parent);

/*
				Функции сравнения
*/

/*
	Функция сравнивает строку str с паттерном pattern
	Если result не NULL, то туда запишуться дополнительные результаты сравнения (например данные по группам)
	После, структуру result необходимо очистить соответствующей функцией
	Возвращает:
			позицию после последнего совпавшего символа последней совпавшей строки		OK
			-1																		ERR
*/
long long check_str_rexpr_object(rexpr_object * pattern, char * str, bytes_t str_len, rexpr_object_result * result);

/*
	Инициализирует структуру result.
	get_next_str и data могут быть NULL
	Возвращает:
			-1		ERR
			0		OK
*/
char init_rexpr_object_result(rexpr_object_result * result, 
				void (* get_next_str)(char ** str, ssize_t * start, ssize_t * end, unsigned int * line, void * get_str_data),
				void * data);

/*
	Очищает структуру result
*/
void clear_rexpr_object_result(rexpr_object_result * result);
#endif