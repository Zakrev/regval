#ifndef __TE_UTF_8_H__
#define __TE_UTF_8_H__

/*
	Определяет размер символа, по первому байту
	Возвращает размер символа, либо 0
*/
unsigned char get_utf8_letter_size(unsigned char ch);

/*
	Подсчитывает количество символов UTF-8 в строке.
	Символы не относящиеся к кодировке считаются как единичные
	Если ascii_len не NULL, то в него помещается размер строки в байтах
*/
unsigned long long get_utf8_strlen(char * str, unsigned long long * ascii_len);

/*
	Переводит длину строки из utf-8 в битовую
	Символы не относящиеся к кодировке считаются как единичные
*/
unsigned long long convert_utf8_to_ascii_strlen(char * str, unsigned long long utf8_len);

/*
	Переводит длину строки из битовой в utf-8
	Символы не относящиеся к кодировке считаются как единичные
*/
unsigned long long convert_ascii_to_utf8_strlen(char * str, unsigned long long ascii_len);

#endif