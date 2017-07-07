#include "rexpr.h"

static char rexpr_object_type_to_ch[rexpr_object_type_end_ch + 1] = {'.', '*', '+', ')', '(', ']', '[', '|', '/'};

static int rexpr_object_type_to_int(char ch)
{
        int i;
        for(i = rexpr_object_type_start_ch; i <= rexpr_object_type_end_ch; i++){
                if(rexpr_object_type_to_ch[i] == ch)
                        return i;
        }
        return rexpr_object_type_unknown_ch;
}

static int parse_rexpr_object_is_text(char * opt, size_t start, size_t pos)
{
        /*
                Возвращает '1', если перед символом в позиции 'pos' стоит 'rexpr_object_type_escape_ch'
                Иначе '0'
        */
        if( (pos - 1) < start )
                return 0;
        if(opt[pos - 1] != rexpr_object_type_to_ch[rexpr_object_type_escape_ch])
                return 0;
                
        return 1;
}

static int parse_rexpr_object_create_STRING(rexpr_object * ro, char * opt, size_t start, size_t * end)
{
        if(ro == NULL){
                PINF("parent is NULL");
                return 1;
        }
        if(opt == NULL){
                PINF("opt is NULL");
                return 1;
        }
        if(start > *end){
                PINF("start > end");
                return 1;
        }
        size_t start_str;
        size_t esc_count;
        size_t idx, idx2;
        
        start_str = *end + 1;
        esc_count = 0;
        while( (start_str - 1) >= start ){
                if(rexpr_object_type_to_int(opt[start_str - 1]) != rexpr_object_type_STRING){
                        if(0 == parse_rexpr_object_is_text(opt, start, start_str - 1))
                                break;
                        esc_count += 1;
                        start_str -= 2;
                } else
                        start_str -= 1;
        }
        ro->str = malloc(sizeof(rexpr_object_str));
        if(ro->str == NULL){
                PERR("rexpr_object_str: malloc()");
                return 1;
        }
        ro->str->len = (*end - start_str + 1) - esc_count;
        ro->str->str = malloc(sizeof(char) * ro->str->len);
        if(ro->str->str == NULL){
                PERR("rexpr_object_str->str: malloc()");
                return 1;
        }
        for(idx = 0, idx2 = start_str; idx2 <= *end; idx2++, idx++){
                if(opt[idx2] == rexpr_object_type_to_ch[rexpr_object_type_escape_ch])
                        idx2 += 1;
                ro->str->str[idx] = opt[idx2];
        }
        *end = start_str - 1;
        
        ro->type = rexpr_object_type_STRING;
        
        return 0;
}

static int parse_rexpr_object(rexpr_object * parent, char * opt, size_t start, size_t * end)
{
        if(parent == NULL){
                PINF("parent is NULL");
                return 1;
        }
        if(opt == NULL){
                PINF("opt is NULL");
                return 1;
        }
        if(start > *end){
                PINF("start > end");
                return 1;
        }
        rexpr_object * ro;
        
        ro = malloc(sizeof(rexpr_object));
        if(ro == NULL){
                PERR("rexpr_object: malloc()");
                return 1;
        }
        ro->child = NULL;
        ro->next = NULL;
        switch(rexpr_object_type_to_int(opt[*end])){
                case rexpr_object_type_DOT:
                        if(1 == parse_rexpr_object_is_text(opt, start, *end)){
                                if(0 != parse_rexpr_object_create_STRING(ro, opt, start, end))
                                        return 1;
                        }
                        break;
                case rexpr_object_type_STAR:
                        if(1 == parse_rexpr_object_is_text(opt, start, *end)){
                                if(0 != parse_rexpr_object_create_STRING(ro, opt, start, end))
                                        return 1;
                        }
                        break;
                case rexpr_object_type_PLUS:
                        if(1 == parse_rexpr_object_is_text(opt, start, *end)){
                                if(0 != parse_rexpr_object_create_STRING(ro, opt, start, end))
                                        return 1;
                        }
                        break;
                case rexpr_object_type_ROUND_BRACKETS_OPEN:
                        if(1 == parse_rexpr_object_is_text(opt, start, *end)){
                                if(0 != parse_rexpr_object_create_STRING(ro, opt, start, end))
                                        return 1;
                        }
                        break;
                case rexpr_object_type_ROUND_BRACKETS_CLOSE:
                        if(1 == parse_rexpr_object_is_text(opt, start, *end)){
                                if(0 != parse_rexpr_object_create_STRING(ro, opt, start, end))
                                        return 1;
                        }
                        break;
                case rexpr_object_type_SQUARE_BRACKETS_OPEN:
                        if(1 == parse_rexpr_object_is_text(opt, start, *end)){
                                if(0 != parse_rexpr_object_create_STRING(ro, opt, start, end))
                                        return 1;
                        }
                        break;
                case rexpr_object_type_SQUARE_BRACKETS_CLOSE:
                        if(1 == parse_rexpr_object_is_text(opt, start, *end)){
                                if(0 != parse_rexpr_object_create_STRING(ro, opt, start, end))
                                        return 1;
                        }
                        break;
                case rexpr_object_type_LINE:
                        if(1 == parse_rexpr_object_is_text(opt, start, *end)){
                                if(0 != parse_rexpr_object_create_STRING(ro, opt, start, end))
                                        return 1;
                        }
                        break;
                case rexpr_object_type_SLASH:
                        if(1 == parse_rexpr_object_is_text(opt, start, *end)){
                                if(0 != parse_rexpr_object_create_STRING(ro, opt, start, end))
                                        return 1;
                        }
                        break;
                case rexpr_object_type_STRING:
                        if(0 != parse_rexpr_object_create_STRING(ro, opt, start, end))
                                return 1;
                        break;
        }
        ro->next = parent->child;
        parent->child = ro;
        
        return 0;
}

size_t rexpr_find(char * str, size_t str_len, char * opt, size_t opt_len, size_t * end_find_ch)
{
        /*
                str     - строка
                opt     - регулярное выражение
                *size   - размер найденой подстроки
                Функция возвращает смещение от начала строки первого символа найденой подстроки
        */
        rexpr_object ro_main;
        size_t end = opt_len;
        
        if(0 != parse_rexpr_object(&ro_main, opt, 0, &end))
                return 0;
        
        return 0;
}