#include "rexpr.h"

static char rexpr_object_type_to_ch[rexpr_object_type_end_ch + 1] = {'.', '*', '+', ')', '(', ']', '[', '/'};

static int rexpr_object_type_to_int(char ch)
{
        int i;
        for(i = rexpr_object_type_start_ch; i <= rexpr_object_type_end_ch; i++){
                if(rexpr_object_type_to_ch[i] == ch)
                        return i;
        }
        return rexpr_object_type_unknown_ch;
}

static int parse_rexpr_object(rexpr_object * parent, char * opt, ssize_t start, ssize_t * end);
static int parse_rexpr_object_is_text(char * opt, ssize_t start, ssize_t pos)
{
        /*
                Возвращает '1', если символ в позиции 'pos' экранируется
                Иначе '0'
        */
        if( (pos - 1) < start )
                return 0;
        if(opt[pos - 1] == rexpr_object_type_to_ch[rexpr_object_type_escape_ch]){
                if( (pos - 2) < start )
                        return 1;
                if(opt[pos - 2] != rexpr_object_type_to_ch[rexpr_object_type_escape_ch])
                        return 1;
        }
        return 0;
}

static int parse_rexpr_object_create_STRING(rexpr_object * parent, char * opt, ssize_t start, ssize_t * end)
{
        PRINT("create_STRING:%lu - %ld\n", start, *end);
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
        ssize_t start_str;
        size_t esc_count;
        size_t idx, idx2;
        rexpr_object * ro;
        
        ro = malloc(sizeof(rexpr_object));
        if(ro == NULL){
                PERR("rexpr_object: malloc()");
                return 1;
        }
        ro->child = NULL;
        ro->next = NULL;
        
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
        ro->data.str.len = (*end - start_str + 1) - esc_count;
        ro->data.str.str = malloc(sizeof(char) * ro->data.str.len);
        if(ro->data.str.str == NULL){
                PERR("rexpr_object_str->str: malloc()");
                return 1;
        }
        for(idx = 0, idx2 = start_str; idx2 <= *end; idx2++, idx++){
                if(opt[idx2] == rexpr_object_type_to_ch[rexpr_object_type_escape_ch])
                        idx2 += 1;
                ro->data.str.str[idx] = opt[idx2];
        }
        *end = start_str - 1;
        
        ro->type = rexpr_object_type_STRING;
        
        ro->next = parent->child;
        parent->child = ro;
        
        return 0;
}

static int parse_rexpr_object_create_DOT(rexpr_object * parent, char * opt, ssize_t start, ssize_t * end)
{
        PRINT("create_DOT:%lu - %ld\n", start, *end);
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
        
        ro->type = rexpr_object_type_DOT;
        *end -= 1;
        
        ro->next = parent->child;
        parent->child = ro;
        
        return 0;
}

static int parse_rexpr_object_create_STAR(rexpr_object * parent, char * opt, ssize_t start, ssize_t * end)
{
        PRINT("create_STAR:%lu - %ld\n", start, *end);
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
        
        switch(parent->type){
                case rexpr_object_type_start_main:
                case rexpr_object_type_ROUND_BRACKETS_OPEN:
                        ro->type = rexpr_object_type_STAR;
                        *end -= 1;
                        ro->next = parent->child;
                        parent->child = ro;
                        
                        return parse_rexpr_object(ro, opt, start, end);
                default:
                        return 1;
        }
        
        
        return 1;
}

static int parse_rexpr_object_create_PLUS(rexpr_object * parent, char * opt, ssize_t start, ssize_t * end)
{
        PRINT("create_PLUS:%lu - %ld\n", start, *end);
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
        
        switch(parent->type){
                case rexpr_object_type_start_main:
                case rexpr_object_type_ROUND_BRACKETS_OPEN:
                        ro->type = rexpr_object_type_PLUS;
                        *end -= 1;
                        ro->next = parent->child;
                        parent->child = ro;
                        
                        return parse_rexpr_object(ro, opt, start, end);
                default:
                        return 1;
        }
        
        
        return 1;
}

static int parse_rexpr_object_create_ROUND_BRACKETS_OPEN(rexpr_object * parent, char * opt, ssize_t start, ssize_t * end)
{
        PRINT("create_ROUND_BRACKETS_OPEN:%lu - %ld\n", start, *end);
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
        
        switch(parent->type){
                case rexpr_object_type_start_main:
                case rexpr_object_type_STAR:
                case rexpr_object_type_PLUS:
                case rexpr_object_type_ROUND_BRACKETS_OPEN:
                        ro->type = rexpr_object_type_ROUND_BRACKETS_OPEN;
                        ro->data.type = rexpr_object_type_start_main;
                        *end -= 1;
                        
                        while(ro->data.type != rexpr_object_type_ROUND_BRACKETS_CLOSE)
                                if(1 == parse_rexpr_object(ro, opt, start, end))
                                        return 1;
                        ro->next = parent->child;
                        parent->child = ro;
                        return 0;
                default:
                        return 1;
        }
        
        return 1;
}

static int parse_rexpr_object_create_ROUND_BRACKETS_CLOSE(rexpr_object * parent, char * opt, ssize_t start, ssize_t * end)
{
        PRINT("create_ROUND_BRACKETS_CLOSE:%lu - %ld\n", start, *end);
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
        
        switch(parent->type){
                case rexpr_object_type_ROUND_BRACKETS_OPEN:
                        parent->data.type = rexpr_object_type_ROUND_BRACKETS_CLOSE;
                        *end -= 1;
                        return 0;
                default:
                        return 1;
        }
        
        return 1;
}

static int parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(rexpr_object * parent, char l, char r)
{
        PRINT("create_SQUARE_BRACKETS_OPEN_create_ch:%c - %c\n", l, r);
        if(parent == NULL){
                PINF("parent is NULL");
                return 1;
        }
        struct rexpr_object_ch_range * ch_range;
        
        ch_range = malloc(sizeof(struct rexpr_object_ch_range));
        if(ch_range == NULL){
                PERR("struct rexpr_object_ch_range: malloc()");
                return 1;
        }
        ch_range->l = l;
        ch_range->r = r;
        ch_range->next = NULL;
        if(parent->data.ch_range == NULL){
                parent->data.ch_range = ch_range;
        } else {
                ch_range->next = parent->data.ch_range;
                parent->data.ch_range = ch_range;
        }
        return 0;
}

static int parse_rexpr_object_create_SQUARE_BRACKETS_OPEN(rexpr_object * parent, char * opt, ssize_t start, ssize_t * end)
{
        PRINT("create_SQUARE_BRACKETS_OPEN:%lu - %ld\n", start, *end);
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
        
        switch(parent->type){
                case rexpr_object_type_start_main:
                case rexpr_object_type_STAR:
                case rexpr_object_type_PLUS:
                case rexpr_object_type_ROUND_BRACKETS_OPEN:
                        ro->type = rexpr_object_type_SQUARE_BRACKETS_OPEN;
                        *end -= 1;
                        
                        while( *end >= start ){
                                if(opt[*end] == rexpr_object_type_to_ch[rexpr_object_type_SQUARE_BRACKETS_CLOSE]){
                                        if(0 == parse_rexpr_object_is_text(opt, start, *end)){
                                                *end -= 1;
                                                ro->next = parent->child;
                                                parent->child = ro;
                                                return 0;
                                        }
                                        if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, opt[*end], '\0'))
                                                return 1;
                                        *end -= 2;
                                        continue;
                                }
                                if((*end - 2) >= start){
                                        if(opt[*end - 1] == '-'){
                                                if(opt[*end - 2] == rexpr_object_type_to_ch[rexpr_object_type_SQUARE_BRACKETS_CLOSE]){
                                                        if(0 == parse_rexpr_object_is_text(opt, start, *end - 2)){
                                                                if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, opt[*end], '\0'))
                                                                        return 1;
                                                                if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, opt[*end - 1], '\0'))
                                                                        return 1;
                                                                *end -= 3;
                                                                ro->next = parent->child;
                                                                parent->child = ro;
                                                                return 0;
                                                        }
                                                        if(opt[*end - 2] > opt[*end])
                                                                return 1;
                                                        if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, opt[*end - 2], opt[*end]))
                                                                return 1;
                                                        *end -= 4;
                                                        continue;
                                                } else {
                                                        if(opt[*end - 2] > opt[*end])
                                                                return 1;
                                                        if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, opt[*end - 2], opt[*end]))
                                                                return 1;
                                                        *end -= 3;
                                                        continue;
                                                }
                                        }
                                }
                                if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, opt[*end], '\0'))
                                        return 1;
                                *end -= 1;
                        }
                        break;
                default:
                        return 1;
        }
        
        return 1;
}

static int parse_rexpr_object(rexpr_object * parent, char * opt, ssize_t start, ssize_t * end)
{
        if(parent == NULL){
                PINF("parent is NULL");
                return 1;
        }
        if(opt == NULL){
                PINF("opt is NULL");
                return 1;
        }
        switch(parent->type){
                case rexpr_object_type_start_main:
                case rexpr_object_type_STAR:
                case rexpr_object_type_PLUS:
                case rexpr_object_type_ROUND_BRACKETS_OPEN:
                case rexpr_object_type_SQUARE_BRACKETS_OPEN:
                        if(start > *end){
                                PINF("start > end");
                                return 1;
                        }
                        break;
        }
        
        while(1){
                PRINT("parse_rexpr_object: parent: %c\n", rexpr_object_type_to_ch[parent->type]);
                switch(rexpr_object_type_to_int(opt[*end])){
                        case rexpr_object_type_DOT:
                                if(1 == parse_rexpr_object_is_text(opt, start, *end)){
                                        if(0 != parse_rexpr_object_create_STRING(parent, opt, start, end))
                                                return 1;
                                } else
                                        if(0 != parse_rexpr_object_create_DOT(parent, opt, start, end))
                                                return 1;
                                break;
                        case rexpr_object_type_STAR:
                                if(1 == parse_rexpr_object_is_text(opt, start, *end)){
                                        if(0 != parse_rexpr_object_create_STRING(parent, opt, start, end))
                                                return 1;
                                } else
                                        if(0 != parse_rexpr_object_create_STAR(parent, opt, start, end))
                                                return 1;
                                break;
                        case rexpr_object_type_PLUS:
                                if(1 == parse_rexpr_object_is_text(opt, start, *end)){
                                        if(0 != parse_rexpr_object_create_STRING(parent, opt, start, end))
                                                return 1;
                                } else
                                        if(0 != parse_rexpr_object_create_PLUS(parent, opt, start, end))
                                                return 1;
                                break;
                        case rexpr_object_type_ROUND_BRACKETS_OPEN:
                                if(1 == parse_rexpr_object_is_text(opt, start, *end)){
                                        if(0 != parse_rexpr_object_create_STRING(parent, opt, start, end))
                                                return 1;
                                } else
                                        if(0 != parse_rexpr_object_create_ROUND_BRACKETS_OPEN(parent, opt, start, end))
                                                return 1;
                                break;
                        case rexpr_object_type_ROUND_BRACKETS_CLOSE:
                                if(1 == parse_rexpr_object_is_text(opt, start, *end)){
                                        if(0 != parse_rexpr_object_create_STRING(parent, opt, start, end))
                                                return 1;
                                } else
                                        if(0 != parse_rexpr_object_create_ROUND_BRACKETS_CLOSE(parent, opt, start, end))
                                                return 1;
                                break;
                        case rexpr_object_type_SQUARE_BRACKETS_OPEN:
                                if(1 == parse_rexpr_object_is_text(opt, start, *end)){
                                        if(0 != parse_rexpr_object_create_STRING(parent, opt, start, end))
                                                return 1;
                                } else
                                        if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN(parent, opt, start, end))
                                                return 1;
                                break;
                        case rexpr_object_type_SQUARE_BRACKETS_CLOSE:
                                if(1 == parse_rexpr_object_is_text(opt, start, *end)){
                                        if(0 != parse_rexpr_object_create_STRING(parent, opt, start, end))
                                                return 1;
                                } else
                                        return 1;
                                break;
                        case rexpr_object_type_SLASH:
                        case rexpr_object_type_STRING:
                                if(0 != parse_rexpr_object_create_STRING(parent, opt, start, end))
                                        return 1;
                                break;
                }
                if(parent->type == rexpr_object_type_start_main){
                        if(*end < start)
                                return 0;
                } else
                        break;
        }
        
        return 0;
}

ssize_t rexpr_find(char * str, ssize_t str_len, char * opt, ssize_t opt_len, ssize_t * end_find_ch)
{
        /*
                str     - строка
                opt     - регулярное выражение
                *size   - размер найденой подстроки
                Функция возвращает смещение от начала строки первого символа найденой подстроки
        */
        rexpr_object ro_main;
        ssize_t end = opt_len - 1;
        
        ro_main.type = rexpr_object_type_start_main;
        ro_main.next = NULL;
        ro_main.child = NULL;
        
        if(0 != parse_rexpr_object(&ro_main, opt, 0, &end))
                return -1;
        *end_find_ch = str_len - 1;
        
        return 0;
}