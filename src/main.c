#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "rexpr.h"

int main(int args, char ** arg)
{
        char * expr = "iam[c1*]";
        char * str = "hello1 iam1superstar";
        ssize_t start, end;
        int fd;
        
        switch(args){
                case 1:
                        start = rexpr_find(str, 20, expr, 8, &end);
                        if(start == -1)
                                PRINT("ERROR\n");
                        break;
                case 3:
                        fd = open(arg[2], O_RDONLY);
                        if(fd == -1){
                                perror("open file");
                                printf("args: <expression> <file_name>\n");
                                return 1;
                        }
                        close(fd);
                        break;
                default:
                        printf("args: <expression> <file_name>\n");
                        return 1;
        }

        return 0;
}