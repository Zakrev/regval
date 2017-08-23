#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../core/rexpr.h"

typedef struct gns_data _gns_data;
struct gns_data {
	unsigned int size;
	char ** lines;
};

void _get_next_str(char ** str, ssize_t * start, ssize_t * end, unsigned int * line, void * get_next_str_data)
{
	/*
		Пример функции выдачи новой строки
		
		При вызове этой функции параметр *line будет хранить актуальную на момент вызова строку
		Функция должна уметь переходить на новую строку из любой позиции *line
	*/
	_gns_data * data = (_gns_data *)get_next_str_data;
	
	if(data == NULL)
		return;
	if(*line + 1 >= data->size)
		return;

	*line += 1;
	*str = data->lines[*line];
	*start = 0;
	*end = strlen(*str) - 1;
}

int main()
{
	rexpr_object expr;
	int ret;
	long long end;
	rexpr_object_result res;
	_gns_data gns_data;
	//char * pattern = "He";
	//char * pattern = "He{1,2}(l)";
	//char * pattern = "He<gr1>(l)<gr1>o";
	//char * pattern = "(.^(o))*";
	//char * pattern = "H|((<e>ll<o>)<e>(e)<o>(o)(<o>r)(<o>{1,3}(.)))*";
	//char * pattern = "H|((<e>ll<o>)<e>(e)<o>(o)(<o>r)({0,1}(<o>){1,10}(^(!).)))*";
	char * pattern = "<e><e><e>Hello";	//проверить в multiline


	char * strs[] = {"Hel", "l", "o Worl", "d", "!"};
	unsigned int strs_size = sizeof(strs) / sizeof(char *);
	
	ret = (int)compile_rexpr(&expr, (uchar_t *)pattern, strlen(pattern));
	if(ret > 0){
		write(1, pattern, ret + 1);
	}
	printf("\nCompile result: %d/%d\n", ret, (int)strlen(pattern) - 1);
	
	gns_data.lines = strs;
	gns_data.size = strs_size;
	init_rexpr_object_result(&res, _get_next_str, &gns_data);

	if(((int)strlen(pattern) - 1) == ret){
		end = check_str_rexpr_object(&expr, strs[0], strlen(strs[0]), &res);
		if(end >= 0){
			unsigned int idx = 0;
			while((void *)res.str != (void *)strs[idx]){
				write(1, strs[idx], strlen(strs[idx]));
				idx++;
			}
			write(1, res.str, res.start);
		}
			
		printf("\nResult: %lld\n", end);
	}
	free_rexpr(&expr);
	
	return 0;
}