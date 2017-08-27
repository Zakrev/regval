#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "../core/rexpr.h"
#include "../encoding/utf_8.h"

#define STRING_BUFF_SIZE 1024
#define PRINT_BUFF_SIZE 1024

typedef struct strs_info strs_info;
struct strs_info {
	int fd;								//файловый дескриптор
	char buffer[STRING_BUFF_SIZE];	//буфер, содержит "строку"
	ssize_t fend;						//смещение последнего байта в файле
};

void _get_str_by_idx(char ** str, ssize_t * start, ssize_t * end, unsigned int line, void * get_str_data)
{
	strs_info * sinf = (strs_info *)get_str_data;

	if(sinf == NULL){
		*str = NULL;
		return;
	}
	if((line * STRING_BUFF_SIZE) > sinf->fend){
		*str = NULL;
		return;
	}
	ssize_t bstart = line * STRING_BUFF_SIZE;
	if(-1 == lseek(sinf->fd, bstart, SEEK_SET)){
		perror("lseek");
		*str = NULL;
		return;
	}

	ssize_t blen = read(sinf->fd, sinf->buffer, STRING_BUFF_SIZE);
	if(blen == -1){
		perror("read");
		*str = NULL;
		return;
	}
	*str = sinf->buffer;
	*start = 0;
	*end = blen - 1;
}

static ssize_t open_file(char * fname, strs_info * sinf)
{
	/*
		Открывает файл.
		Возвращает длину первой строки, либо -1.
	*/
	int fd;

	fd = open(fname, O_RDONLY);
	if(-1 == fd){
		perror("open");
		return -1;
	}
	sinf->fd = fd;
	sinf->fend = lseek(fd, 0, SEEK_END);
	if(sinf->fend == -1){
		perror("lseek");
		return -1;
	}
	sinf->fend -= 1;
	if(sinf->fend <= 0){
		fprintf(stderr, "EOL\n");
		return -1;
	}
	lseek(fd, 0, SEEK_SET);
	ssize_t blen = read(sinf->fd, sinf->buffer, STRING_BUFF_SIZE);
	if(blen == -1){
		perror("read");
		return -1;
	}

	return blen;
}

static void print_str(int fd, ssize_t start, ssize_t end)
{
	fprintf(stderr, "found s/e: %lld / %lld\n", (long long)start, (long long)end);
	char buff[PRINT_BUFF_SIZE];
	if(-1 == lseek(fd, start, SEEK_SET))
		return;

	write(1, "'", 1);
	while(1){
		if((end - start) + 1 > PRINT_BUFF_SIZE){
			if(PRINT_BUFF_SIZE != read(fd, buff, PRINT_BUFF_SIZE))
				return;
			write(1, buff, PRINT_BUFF_SIZE);
			start += PRINT_BUFF_SIZE;
			continue;
		}
		if((end - start) + 1 != read(fd, buff, (end - start) + 1))
			return;
		write(1, buff, (end - start) + 1);
		break;
	}
	write(1, "'\n", 2);
}

int main(int args, char ** arg)
{
	if(args != 3){
		fprintf(stderr, "%s <pattern> <filename>\n", arg[0]);
		return 1;
	}
	rexpr_object expr;
	rexpr_object_result result;
	strs_info sinf;
	int ret;
	ssize_t str_len = 0;

	str_len = open_file(arg[2], &sinf);
	if(str_len == -1)
		return 1;

	ret = (int)compile_rexpr(&expr, (uchar_t *)arg[1], strlen(arg[1]));
	if(ret != strlen(arg[1]) - 1){
		write(2, "error on compile: ", strlen("error on compile: "));
		write(2, arg[1], ret + 1);
		write(2, "\x1b[41m", strlen("\x1b[41m"));
		write(2, arg[1] + ret + 1, 1);
		write(2, "\x1b[0m", strlen("\x1b[0m"));
		write(2, arg[1] + ret + 2, strlen(arg[1]) - (ret + 1));
		write(2, "\n", 1);
		free_rexpr(&expr);
		return 1;
	}

	init_rexpr_object_result(&result, sinf.buffer, str_len, _get_str_by_idx, &sinf);

	unsigned int line = 0;
	ssize_t start = -1;
	while(1){
		ssize_t end = (ssize_t)check_str_rexpr_object(&expr, &result);

		if(end != -1){
			start += 1;
			fprintf(stderr, "result: %lld\n", (long long)end);
			print_str(sinf.fd, line * STRING_BUFF_SIZE + start, result.line * STRING_BUFF_SIZE + end);
			fprintf(stderr, "\n");
			start = result.start + 1;
			line = result.line;
			result.start += 1;
		} else {
			result.start += 1;
			start += 1;
		}
		if( ((result.line * STRING_BUFF_SIZE) + result.start) >= sinf.fend){
			break;
		}
	}

	clear_rexpr_object_result(&result);
	free_rexpr(&expr);

	return 0;
}
