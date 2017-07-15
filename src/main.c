#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#include "rexpr.h"

int main(int args, char ** arg)
{
        char * opt = "[ \n][a-zA-z']+";
        char * str = "Hello, I'am superstar.\nAnd I'am really cool.\n";
        char tmp[256];
        ssize_t start, end, stmp;
        int fd;
        char * file;
        ssize_t file_size;
        
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
                        printf("expression: %s\nfilename: %s\n", arg[1], arg[2]);
                        fd = open(arg[2], O_RDONLY);
                        if(fd == -1){
                                perror("open file");
                                printf("args: <expression> <file_name>\n");
                                return 1;
                        }
                        file_size = lseek(fd, 0, SEEK_END);
                        if(file_size == -1)
                                return 1;
                        lseek(fd, 0, SEEK_SET);
                        file = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
                        if((void *)file == (void *)-1)
                                return 1;
                        stmp = start = 0;
                        while(stmp >= 0){
                                stmp = rexpr_find(file + start, file_size - start, arg[1], strlen(arg[1]), &end);
                                if(stmp >= 0){
                                        end += start;
                                        start += stmp;
                                        memcpy(tmp, file + start, end - start + 1);
                                        tmp[end - start + 1] = '\0';
                                        printf("\tFOUND:%lld\n\'%s\'\n", (long long)(end - start + 1),tmp);
                                        start += 1;
                                }
                        }
                        munmap(file, file_size);
                        close(fd);
                        break;
                default:
                        printf("args: <expression> <file_name>\n");
                        return 1;
        }

        return 0;
}