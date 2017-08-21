#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../core/rexpr.h"

int main()
{
	rexpr_object expr;
	int ret;
	long long end;
	//char * pattern = "He{1,2}(l)";					//нет утечек
	//char * str = "HelloWorld!";
	//char * pattern = "He<gr1>(l)<gr1>o";					//нет утечек
	//char * str = "HelloWorld!";
	//char * pattern = "(^(o).)*";						//нет утечек
	//char * str = "HelloWorld!";
	//char * pattern = "H|((<e>ll<o>)<e>(e)<o>(o)(<o>r)(<o>{1,3}(.)))*";	//нет утечек
	//char * str = "HelloWorld!";
	
	ret = (int)compile_rexpr(&expr, (uchar_t *)pattern, strlen(pattern));
	if(ret > 0){
		write(1, pattern, ret + 1);
	}
	printf("\nCompile result: %d/%d\n", ret, (int)strlen(pattern) - 1);
	if(((int)strlen(pattern) - 1) == ret){
		end = check_str_rexpr_object(&expr, str, strlen(str), NULL);
		if(end > 0)
			write(1, str, end);
		printf("\nResult: %lld\n", end);
	}
	free_rexpr(&expr);
	
	return 0;
}