#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#include "../rexpr.h"
#include "../debug.h"

#define ALL_MATCHES 0   //1 - все подстроки

typedef struct multiline_data multiline_data;
struct multiline_data {
        unsigned int idx;
        ssize_t start;
        
        unsigned int size;
        char ** strs;
        ssize_t ** info;
};

void rexpr_find_get_next_str(char ** str, ssize_t * start, ssize_t * end, void * data)
/*void (* get_next_str)(char ** str, ssize_t * start, ssize_t * end, void * data)*/
{
        if(data == NULL){
                *str = NULL;
                return;
        }
        multiline_data * mdata = (multiline_data *)data;
        
        mdata->idx += 1;
        if(mdata->idx >= mdata->size){
                *str = NULL;
                return;
        }
        *str = mdata->strs[mdata->idx];
        *start = mdata->info[mdata->idx][0];
        *end = mdata->info[mdata->idx][1];
}

typedef struct state_data state_data;
struct state_data {
        int st_idx;
        ssize_t st_start;        
        unsigned int en_idx;
        ssize_t en_end;
};

state_data rexpr_find_multiline(multiline_data * mdata, const char * opt, ssize_t opt_end)
{
        /*
                Функция возвращает информацию о найденой подстроке
                Если state_data.st_idx < 0, значит подстрока не найдена
        */
        rexpr_object ro_main;
        state_data state;
        
        ro_main.type = rexpr_object_type_start_main;
        ro_main.next = NULL;
        ro_main.child = NULL;
        bzero(&state, sizeof(state));
        
        if(0 != parse_rexpr_object(&ro_main, opt, 0, &opt_end)){
                printf("Expression error on symbol %lld: %s\n", (long long)(opt_end + 1), opt);
                free_rexpr_objects(&ro_main);
                state.st_idx = -1;
                return state;
        }
        
        ssize_t start = mdata->start;
        state.st_idx = mdata->idx;
        while(1){
                char * str = mdata->strs[mdata->idx];
                ssize_t end = mdata->info[mdata->idx][1];
                switch(check_str_rexpr_object(&ro_main, str, start, &end, rexpr_find_get_next_str, mdata)){
                        case rexpr_check_status_SUCCESS:
                                state.st_start = start;
                                state.en_idx = mdata->idx;
                                state.en_end = end;
                                free_rexpr_objects(&ro_main);
                                return state;
                        default:
                                start += 1;
                                if(start > mdata->info[mdata->idx][1]){
                                        mdata->idx += 1;
                                        if(mdata->idx >= mdata->size){
                                                state.st_idx = -1;
                                                return state;
                                        }
                                        start = mdata->info[mdata->idx][0];
                                        state.st_idx = mdata->idx;
                                }
                }
        }
        free_rexpr_objects(&ro_main);
        
        state.st_idx = -1;
        return state;
}

int main(int args, char ** arg)
{
        /*
                Пример использования
        */
        char * opt = "[а-яА-Яa-zA-Z]+";
        char *str[4] = {"Привет, к", "ак дела?", " Hello World! Ho", "w are you?"};
        state_data state;
        multiline_data mdata;
        
        switch(args){
                case 1:
                        mdata.idx = 0;
                        mdata.start = 0;
                        mdata.strs = str;
                        mdata.size = 4;
                        ssize_t i, j;
                        mdata.info = malloc(sizeof(ssize_t *) * 4);
                        for(i = 0; i < 4; i++){
                                mdata.info[i] = malloc(sizeof(ssize_t) * 2);
                                mdata.info[i][0] = 0;
                                mdata.info[i][1] = strlen(mdata.strs[i]) - 1;
                        }
                        while(1){
                                state = rexpr_find_multiline(&mdata, opt, strlen(opt) - 1);
                                
                                if(state.st_idx < 0)
                                        break;
                                printf("FOUND (line/start/line/end): %u/%lld/%u/%lld: \'", 
                                                state.st_idx, 
                                                (long long)state.st_start,
                                                state.en_idx,
                                                (long long)state.en_end);
                                if(state.st_idx == state.en_idx){
                                        j = state.st_idx;
                                        for(i = state.st_start; i <= state.en_end; i++){
                                                printf("%c", mdata.strs[j][i]);
                                        }
                                } else {
                                        j = state.st_idx;
                                        for(i = state.st_start; i <= mdata.info[j][1]; i++){
                                                printf("%c", mdata.strs[j][i]);
                                        }
                                        for(j = state.st_idx + 1; j < state.en_idx; j++){
                                                for(i = mdata.info[j][0]; i <= mdata.info[j][1]; i++){
                                                        printf("%c", mdata.strs[j][i]);
                                                }
                                        }
                                        j = state.en_idx;
                                        for(i = mdata.info[j][0]; i <= state.en_end; i++){
                                                printf("%c", mdata.strs[j][i]);
                                        }
                                }
                                printf("\'\n");
#if ALL_MATCHES == 1
                                        mdata.idx = state.st_idx;
                                        mdata.start = state.st_start + 1;
                                        if(mdata.start > mdata.info[mdata.idx][1]){
                                                mdata.idx += 1;
                                                if(mdata.idx >= mdata.size)
                                                        break;
                                                mdata.start = 0;
                                        }
#else
                                        mdata.start = state.en_end + 1;
                                        if(mdata.start > mdata.info[mdata.idx][1]){
                                                mdata.idx += 1;
                                                if(mdata.idx >= mdata.size)
                                                        break;
                                                mdata.start = 0;
                                        }
#endif
                        }
                        break;
                default:
                        printf("args: <expression> <file_name>\n");
                        return 1;
        }

        return 0;
}