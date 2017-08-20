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
	char * pattern = "He{1,2}(l)";
	char * str = "HelloWorld!";
	
	ret = (int)compile_rexpr(&expr, (uchar_t *)pattern, strlen(pattern));
	printf("\nCompile result: %d/%d\n", ret, (int)strlen(pattern) - 1);
	if(((int)strlen(pattern) - 1) == ret){
		end = check_str_rexpr_object(&expr, str, strlen(str), NULL);
		printf("\nResult: %lld: ", end);
		if(end > 0)
			write(1, str, end);
		printf("\n");
	}
	free_rexpr(&expr);
	
	return 0;
}