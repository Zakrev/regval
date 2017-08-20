#ifndef _REXPR_TYPES_H_
#define _REXPR_TYPES_H_

#include "rexpr.h"

/*
	Определение типов

	Основные символы:
	символ		описание
	.		один любой символ
	*		повторение 0 или более раз, нельзя использовать с: '^()', '{l,r}()', '*', '+'
	+		повторение 1 или более раз, нельзя использовать с: '^()', '{l,r}()', '*', '+'
	()		группа правил
	[]		один любой символ из набора, внутри скобок все интерпретируется как символы, экранирований не нужно
	<>		обращение к именованной группе
	TODO

	Дополнительные символы:
	дополнительные символы вместе не используются, 
	например "{l,r}|()" будет восприниматься как: "{l,r}" - строка, "|()" - группа ИЛИ
	символ		связка		описание
	/		[/esc]		начало esc-символа, используется только в [], esc - esc-символ
	|		|()		логическое ИЛИ (OR), |((a)(b)[abc]a*abc) == a|b|[abc]|a*|abc
	^		^()		логическое ОТРИЦАНИЕ (NOT), ^((a)(b)[abc]a*abc) == NOT((a)(b)[abc]a*abc)
	{}		{l,r}()		определение количества повторений группы от l до r раз, 
						где l,r - целые числа в диапазоне [0,65535] (unsigned short), и l <= r
	<>		<name>()		именование группы, где name - любые символы кроме '<' и '>'
	TODO

	esc-символы:
	должны быть однобайтовые
	символ		описание
	n		перевод строки
	t		табуляция
	TODO
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
	rexpr_object_type_ANGLE_BRACKETS_OPEN,
	rexpr_object_type_ANGLE_BRACKETS_CLOSE,
	rexpr_object_type_STRING,
	rexpr_object_type_CH_RANGE,
	
	/*Параметры*/
	rexpr_object_type_start_ch = rexpr_object_type_DOT,
	rexpr_object_type_end_ch = rexpr_object_type_ANGLE_BRACKETS_CLOSE,
	rexpr_object_type_null,
	rexpr_object_type_unknown_ch = rexpr_object_type_STRING
};

enum rexpr_object_type_second {
	/*Типы дополнительных объектов*/
	rexpr_object_type_second_ESCAPE,
	rexpr_object_type_second_NOT,
	rexpr_object_type_second_OR,
	rexpr_object_type_second_BRACE_OPEN,
	rexpr_object_type_second_BRACE_CLOSE,
	rexpr_object_type_second_ANGLE_BRACKETS_OPEN,
	rexpr_object_type_second_ANGLE_BRACKETS_CLOSE,
	
	/*Параметры*/
	rexpr_object_type_second_start_ch = rexpr_object_type_second_ESCAPE,
	rexpr_object_type_second_end_ch = rexpr_object_type_second_ANGLE_BRACKETS_CLOSE,
	rexpr_object_type_second_null,
	rexpr_object_type_second_unknown_ch,
	
	/*Прочее*/
	rexpr_object_type_second_ROUND_BRACKETS_CLOSE
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

#ifdef _REXPR_TYPES_STATIC_FUNC_
/*Функции по работе с символами*/
static uchar_t rexpr_object_type_to_ch[rexpr_object_type_end_ch + 1] = {'.', '*', '+', ')', '(', ']', '[', '>', '<'};
static uchar_t rexpr_object_type_to_int(uchar_t ch)
{
	unsigned int i;
	for(i = rexpr_object_type_start_ch; i <= rexpr_object_type_end_ch; i++){
		if(rexpr_object_type_to_ch[i] == ch)
			return i;
	}
	return rexpr_object_type_unknown_ch;
}

/*Функции по работе с дополнительными символами*/
static uchar_t rexpr_object_type_second_to_ch[rexpr_object_type_second_end_ch + 1] = {'/', '^', '|', '}', '{', '>', '<'};
static uchar_t rexpr_object_type_second_to_int(uchar_t ch)
{
	unsigned int i;
	for(i = rexpr_object_type_second_start_ch; i <= rexpr_object_type_second_end_ch; i++){
		if(rexpr_object_type_second_to_ch[i] == ch)
			return i;
	}
	return rexpr_object_type_second_unknown_ch;
}

/*Функции по работе с esc-символами*/
static uchar_t rexpr_escape_type_to_ch[rexpr_escape_type_end_ch + 1] = {'\n', '\t'};
static uchar_t rexpr_escape_type_to_esc[rexpr_escape_type_end_ch + 1] = {'n', 't'};
static uchar_t rexpr_escape_type_to_int(uchar_t ch)
{
	unsigned int i;
	for(i = rexpr_escape_type_start_ch; i <= rexpr_escape_type_end_ch; i++){
		if(rexpr_escape_type_to_esc[i] == ch)
			return i;
	}
	return rexpr_escape_type_unknown_ch;
}
#endif

#endif