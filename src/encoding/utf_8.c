unsigned char get_utf8_letter_size(unsigned char ch)
{
	/*
		Определяет размер символа, по первому байту
		Возвращает размер символа, либо 0
	*/
	ch = ch >> 4;
	if(ch == 0xf){ //начало четырехбайтового символа UTF-8
		return 4;
	}
	if(ch == 0xe){ //начало трехбайтового символа UTF-8
		return 3;
	}
	ch = ch >> 1;
	if(ch == 0x6){ //начало двухбайтового символа UTF-8
		return 2;
	}
	ch = ch >> 2;
	if(ch == 0x0){ //начало однобайтового символа UTF-8
		return 1;
	}
	return 0;
}

unsigned long long get_utf8_strlen(char * str, unsigned long long * ascii_len)
{
	/*
		Подсчитывает количество символов UTF-8 в строке.
		Символы не относящиеся к кодировке считаются как единичные
		Если ascii_len не NULL, то в него помещается размер строки в байтах
	*/
	if(str == (void *)0)
		return 0;
	
	unsigned long long len = 0;
	unsigned long long offset = 0;
	while(1){
		unsigned char ch = (unsigned char)*(str + offset);
		if(ch == 0){
			if(ascii_len != (void *)0)
				*ascii_len = offset;
			return len;
		}
		int lsize = get_utf8_letter_size(ch);
		if(lsize != 0)
			offset += lsize;
		else
			offset += 1;
		len += 1;
	}
	return len;
}

unsigned long long convert_utf8_to_ascii_strlen(char * str, unsigned long long utf8_len)
{
	/*
		Переводит длину строки из utf-8 в битовую
		Символы не относящиеся к кодировке считаются как единичные
	*/
	if(str == (void *)0)
		return 0;

	unsigned long long offset = 0;
	while(utf8_len > 0){
		unsigned char ch = (unsigned char)*(str + offset);
		if(ch == 0){
			return offset;
		}
		int lsize = get_utf8_letter_size(ch);
		if(lsize != 0)
			offset += lsize;
		else
			offset += 1;
		utf8_len -= 1;
	}
	return offset;
}

unsigned long long convert_ascii_to_utf8_strlen(char * str, unsigned long long ascii_len)
{
	/*
		Переводит длину строки из битовой в utf-8
		Символы не относящиеся к кодировке считаются как единичные
	*/
	if(str == (void *)0)
		return 0;

	unsigned long long len = 0;
	unsigned long long offset = 0;
	while(ascii_len > offset){
		unsigned char ch = (unsigned char)*(str + offset);
		if(ch == 0){
			return len;
		}
		int lsize = get_utf8_letter_size(ch);
		if(lsize != 0){
			offset += lsize;
		} else {
			offset += 1;
		}
		len += 1;
	}
	return len;
}