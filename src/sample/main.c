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

void for_each_group_result(char ** strs, rexpr_object_result * res)
{
	unsigned int g_idx;

	if(res->group_result == NULL)
		return;
	for(g_idx = 0; g_idx < res->group_result_size; g_idx++){
		printf("group: %u\n", g_idx);
		rexpr_object_result_group * g_res = res->group_result[g_idx];
		while(g_res != NULL){
			unsigned int l_idx = g_res->line_start;
			if(l_idx == g_res->line_end){
				//printf("start/end: l/s/e: %d/%d/%d\n", (int)l_idx, (int)g_res->start, (int)g_res->end);
				write(1, "'", 1);
				write(1, strs[l_idx] + g_res->start, (g_res->end - g_res->start) + 1);
				write(1, "'\n", 2);
			} else {
				//printf("start: l/s/e: %d/%d/%d\n", (int)l_idx, (int)g_res->start, (int)strlen(strs[l_idx]) - 1);
				write(1, "'", 1);
				write(1, strs[l_idx] + g_res->start, strlen(strs[l_idx]));
				write(1, "' ", 2);
				for(l_idx = l_idx + 1; l_idx < g_res->line_end; l_idx++){
					//printf("       l/s/e: %d/%d/%d\n", (int)l_idx, 0, (int)strlen(strs[l_idx]) - 1);
					write(1, "'", 1);
					write(1, strs[l_idx], strlen(strs[l_idx]));
					write(1, "' ", 2);
				}
				//printf("end:   l/s/e: %d/%d/%d\n", (int)l_idx, 0, (int)g_res->end);
				write(1, "'", 1);
				write(1, strs[l_idx], (g_res->end - g_res->start) + 1);
				write(1, "'\n", 2);
			}
			g_res = g_res->next;
		}
	}
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
	//char * pattern = "<e><e><e><h>";	//проверить в multiline
	//char * pattern = "[A-Za-z/n/t/0]*";

	//char * pattern = "При";
	//char * pattern = "Пр{1,2}(и)в";
	//char * pattern = "{1,3}(|((П<gr1>(р)ивет ми<gr1>!)([A-Za-z ]*!)))";
	//char * pattern = "(.^(o))*";
	//char * pattern = "[А-Яа-яA-Za-z/n/t/0 !]*";
	//char * pattern = "<1>(<2>([А-Яа-я]*)([ !]*)<3>([A-Za-z]*))<1><1><1>";

	char * pattern = "[А-Яа-я !]*<1>([A-Za-z !]*)";

	char * strs[] = {"При", "вет ", "мир! ", "Hel", "l", "o Worl", "d", "!"};
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
			write(1, "\n", 1);
			for_each_group_result(strs, &res);
		}
			
		printf("\nResult: %lld\n", end);
	}
	free_rexpr(&expr);

	return 0;
}