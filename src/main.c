#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "rexpr.h"

int main(int args, char ** arg)
{
        char * opt = "[][[]";
        char * str = "Hello, I'am su++///per+*s[]t*ar.\nAnd I'am really cool.\n";
        char tmp[256];
        ssize_t start, end, stmp;
        int fd;
        
        switch(args){
                case 1:
                        stmp = start = 0;
                        while(stmp >= 0){
                                stmp = rexpr_find(str + start, strlen(str + start), opt, strlen(opt), &end);
                                if(stmp >= 0){
                                        end += start;
                                        start += stmp;
                                        memcpy(tmp, str + start, end - start + 1);
                                        tmp[end - start + 1] = '\0';
                                        printf("\tFOUND:%lld\n\'%s\'\n", (long long)(end - start + 1),tmp);
                                        start += 1;
                                }
                        }
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