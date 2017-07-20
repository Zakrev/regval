#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#include "rexpr.h"

#define ALL_MATCHES 0   //1 - все подстроки

int main(int args, char ** arg)
{
        /*
                Пример использования
        */
        char * opt = "[а-яА-Яa-zA-Z]+";
        char * str = "Привет мир! Hello world! Как дела? How are you? БellеберDa\n";
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
                                        printf("FOUND (pos/len) %lld/%lld / \'%s\'\n", (long long)start, (long long)(end - start + 1), tmp);
#if ALL_MATCHES == 1
                                        start += 1;
#else
                                        start += end - start + 1;
#endif
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
                                        printf("FOUND:%lld / \'%s\'\n", (long long)(end - start + 1),tmp);
#if ALL_MATCHES == 1
                                        start += 1;
#else
                                        start += end - start + 1;
#endif
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