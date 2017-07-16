#include "rexpr.h"

static char rexpr_escape_type_to_ch[rexpr_escape_type_end_ch + 1] = {'\n', '\t'};
static char rexpr_escape_type_to_esc[rexpr_escape_type_end_ch + 1] = {'n', 't'};
static int rexpr_escape_type_to_int(char ch)
{
        int i;
        for(i = rexpr_escape_type_start_ch; i <= rexpr_escape_type_end_ch; i++){
                if(rexpr_escape_type_to_esc[i] == ch)
                        return i;
        }
        return rexpr_escape_type_unknown_ch;
}

static char rexpr_object_type_to_ch[rexpr_object_type_end_ch + 1] = {'.', '*', '+', ')', '(', ']', '['};
static int rexpr_object_type_to_int(char ch)
{
        int i;
        for(i = rexpr_object_type_start_ch; i <= rexpr_object_type_end_ch; i++){
                if(rexpr_object_type_to_ch[i] == ch)
                        return i;
        }
        return rexpr_object_type_unknown_ch;
}

static int parse_rexpr_object(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end);

static int parse_rexpr_object_create_STRING(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end)
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
        while( (start_str - 1) >= start ){
                if(rexpr_object_type_to_int(opt[start_str - 1]) != rexpr_object_type_STRING)
                        break;
                start_str -= 1;
        }
        ro->data.str.len = *end - start_str + 1;
        ro->data.str.str = malloc(sizeof(char) * ro->data.str.len);
        if(ro->data.str.str == NULL){
                PERR("rexpr_object_str->str: malloc()");
                return 1;
        }
        for(idx = 0, idx2 = start_str; idx2 <= *end; idx2++, idx++){
                ro->data.str.str[idx] = opt[idx2];
        }
        *end = start_str - 1;
        
        ro->type = rexpr_object_type_STRING;
        
        ro->next = parent->child;
        parent->child = ro;

        return 0;
}

static int parse_rexpr_object_create_DOT(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end)
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

static int parse_rexpr_object_create_STAR(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end)
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
                        PINF("unexpected parent type");
                        return 1;
        }
        
        
        return 1;
}

static int parse_rexpr_object_create_PLUS(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end)
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
                        PINF("unexpected parent type");
                        return 1;
        }
        
        
        return 1;
}

static int parse_rexpr_object_create_ROUND_BRACKETS_OPEN(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end)
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
                        PINF("unexpected parent type");
                        return 1;
        }
        
        return 1;
}

static int parse_rexpr_object_create_ROUND_BRACKETS_CLOSE(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end)
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
                        PINF("unexpected parent type");
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
        rexpr_object_ch_range * ch_range;
        
        ch_range = malloc(sizeof(rexpr_object_ch_range));
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

static int parse_rexpr_object_create_SQUARE_BRACKETS_OPEN(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end)
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
                                /*
                                        Символ '*' в следующих коментах, обозначает любой один символ
                                        Символ '+' - текущий
                                */
                                if(opt[*end] == rexpr_object_type_to_ch[rexpr_object_type_SQUARE_BRACKETS_CLOSE]){
                                        /*
                                                Если текущий символ закрывает этот объект
                                        */
                                        if((*end - 1) >= start){
                                                if(opt[*end - 1] == rexpr_object_type_to_ch[rexpr_object_type_SQUARE_BRACKETS_CLOSE]){
                                                        /*
                                                                Если '[['
                                                        */
                                                        if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, opt[*end], '\0'))
                                                                return 1;
                                                        *end -= 1;
                                                }
                                        }
                                        *end -= 1;
                                        ro->next = parent->child;
                                        parent->child = ro;
                                        return 0;
                                }
                                if((*end - 1) >= start){
                                        /* Если '*+' */
                                        if(opt[*end - 1] == rexpr_escape_init){
                                                /* Если '\+' */
                                                if(rexpr_escape_type_to_int(opt[*end]) != rexpr_escape_type_unknown_ch){
                                                        if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(
                                                                        ro, 
                                                                        rexpr_escape_type_to_ch[rexpr_escape_type_to_int(opt[*end])], 
                                                                        '\0'))
                                                                return 1;
                                                        *end -= 2;
                                                        continue;
                                                }
                                        }
                                }
                                if((*end - 2) >= start){
                                        /* Если '**+' */
                                        if(opt[*end - 1] == '-'){
                                                /* Если '*-+' */
                                                if(opt[*end - 2] == rexpr_object_type_to_ch[rexpr_object_type_SQUARE_BRACKETS_CLOSE]){
                                                        /* Если '[-+' */
                                                        if((*end - 3) >= start){
                                                                if(opt[*end - 3] == rexpr_object_type_to_ch[rexpr_object_type_SQUARE_BRACKETS_CLOSE]){
                                                                        /* Если '[[-+' */
                                                                        if(opt[*end - 2] > opt[*end])
                                                                                return 1;
                                                                        if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, opt[*end - 2], opt[*end]))
                                                                                return 1;
                                                                        *end -= 4;
                                                                        ro->next = parent->child;
                                                                        parent->child = ro;
                                                                        return 0;
                                                                }
                                                        }
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
                                                *end -= 3;
                                                continue;
                                        }
                                }
                                if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, opt[*end], '\0'))
                                        return 1;
                                *end -= 1;
                        }
                        break;
                default:
                        PINF("unexpected parent type");
                        return 1;
        }
        
        return 1;
}

static int parse_rexpr_object(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end)
{
        /*
                Функция парсит регулярное выражение, создает его представление в структурах
        */
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
                                if(0 != parse_rexpr_object_create_DOT(parent, opt, start, end))
                                                return 1;
                                break;
                        case rexpr_object_type_STAR:
                                if(0 != parse_rexpr_object_create_STAR(parent, opt, start, end))
                                                return 1;
                                break;
                        case rexpr_object_type_PLUS:
                                if(0 != parse_rexpr_object_create_PLUS(parent, opt, start, end))
                                                return 1;
                                break;
                        case rexpr_object_type_ROUND_BRACKETS_OPEN:
                                if(0 != parse_rexpr_object_create_ROUND_BRACKETS_OPEN(parent, opt, start, end))
                                                return 1;
                                break;
                        case rexpr_object_type_ROUND_BRACKETS_CLOSE:
                                if(0 != parse_rexpr_object_create_ROUND_BRACKETS_CLOSE(parent, opt, start, end))
                                                return 1;
                                break;
                        case rexpr_object_type_SQUARE_BRACKETS_OPEN:
                                if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN(parent, opt, start, end))
                                                return 1;
                                break;
                        case rexpr_object_type_SQUARE_BRACKETS_CLOSE:
                        case rexpr_object_type_STRING:
                                if(0 != parse_rexpr_object_create_STRING(parent, opt, start, end))
                                        return 1;
                                break;
                        default:
                                PINF("unexpected rexpr_object type");
                                return 1;
                }
                if(parent->type == rexpr_object_type_start_main){
                        if(*end < start)
                                return 0;
                } else
                        break;
        }
        
        return 0;
}



static int check_str_rexpr_object_ROUND_BRACKETS_OPEN(rexpr_object * parent, const char * str, ssize_t * start, ssize_t end);
static int check_str_rexpr_object_STAR(rexpr_object * parent, const char * str, ssize_t * start, ssize_t end)
{
        if(parent == NULL){
                PINF("parent is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(parent->child == NULL){
                PINF("child is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(str == NULL){
                PINF("str is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        ssize_t start_S = *start;
        ssize_t start_S1;
        rexpr_object * ro = parent->child;
        rexpr_object_ch_range * ch_range;
        int i;
        
        while(1)
        switch(ro->type){
                case rexpr_object_type_ROUND_BRACKETS_OPEN:
                        if(rexpr_check_status_SUCCESS != check_str_rexpr_object_ROUND_BRACKETS_OPEN(ro, str, &start_S, end))
                                goto break_while;
                        break;
                case rexpr_object_type_SQUARE_BRACKETS_OPEN:
                        if(start_S > end)
                                goto break_while;
                        ch_range = ro->data.ch_range;
                        while(ch_range != NULL){
                                if(ch_range->r == '\0'){
                                        if(ch_range->l == str[start_S]){
                                                start_S += 1;
                                                break;
                                        }
                                } else {
                                        if(ch_range->l <= str[start_S] 
                                                && ch_range->r >= str[start_S]){
                                                start_S += 1;
                                                break;
                                        }
                                }
                                ch_range = ch_range->next;
                        }
                        if(ch_range == NULL)
                                goto break_while;
                        break;
                case rexpr_object_type_STRING:
                        start_S1 = start_S;
                        for(i = 0; i < ro->data.str.len; i++){
                                if(start_S1 > end)
                                        goto break_while;
                                if(ro->data.str.str[i] != str[start_S1])
                                        goto break_while;
                                start_S1 += 1;
                        }
                        start_S = start_S1;
                        break;
                case rexpr_object_type_DOT:
                        if(start_S > end)
                                goto break_while;
                        start_S += 1;
                        break;
                default:
                        PINF("unexpected rexpr_object type");
                        return rexpr_check_status_UNSUCCESS;
        }
        break_while:
        *start = start_S;
        return rexpr_check_status_SUCCESS;
}

static int check_str_rexpr_object_PLUS(rexpr_object * parent, const char * str, ssize_t * start, ssize_t end)
{
        if(parent == NULL){
                PINF("parent is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(parent->child == NULL){
                PINF("child is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(str == NULL){
                PINF("str is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        ssize_t start_S = *start;
        ssize_t start_S1;
        rexpr_object * ro = parent->child;
        rexpr_object_ch_range * ch_range;
        int i;
        
        while(1)
        switch(ro->type){
                case rexpr_object_type_ROUND_BRACKETS_OPEN:
                        if(rexpr_check_status_SUCCESS != check_str_rexpr_object_ROUND_BRACKETS_OPEN(ro, str, &start_S, end))
                                goto break_while;
                        break;
                case rexpr_object_type_SQUARE_BRACKETS_OPEN:
                        if(start_S > end)
                                goto break_while;
                        ch_range = ro->data.ch_range;
                        while(ch_range != NULL){
                                if(ch_range->r == '\0'){
                                        if(ch_range->l == str[start_S]){
                                                start_S += 1;
                                                break;
                                        }
                                } else {
                                        if(ch_range->l <= str[start_S] 
                                                && ch_range->r >= str[start_S]){
                                                start_S += 1;
                                                break;
                                        }
                                }
                                ch_range = ch_range->next;
                        }
                        if(ch_range == NULL)
                                goto break_while;
                        break;
                case rexpr_object_type_STRING:
                        start_S1 = start_S;
                        for(i = 0; i < ro->data.str.len; i++){
                                if(start_S1 > end)
                                        goto break_while;
                                if(ro->data.str.str[i] != str[start_S1])
                                        goto break_while;
                                start_S1 += 1;
                        }
                        start_S = start_S1;
                        break;
                case rexpr_object_type_DOT:
                        if(start_S > end)
                                goto break_while;
                        start_S += 1;
                        break;
                default:
                        PINF("unexpected rexpr_object type");
                        return rexpr_check_status_UNSUCCESS;
        }
        break_while:
        if(*start >= start_S){
                return rexpr_check_status_UNSUCCESS;
        } else {
                *start = start_S;
                return rexpr_check_status_SUCCESS;
        }
        
        return rexpr_check_status_UNSUCCESS;
}

static int check_str_rexpr_object_ROUND_BRACKETS_OPEN(rexpr_object * parent, const char * str, ssize_t * start, ssize_t end)
{
        if(parent == NULL){
                PINF("parent is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(parent->child == NULL){
                PINF("child is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(str == NULL){
                PINF("str is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        ssize_t start_S = *start;
        rexpr_object * ro;
        rexpr_object_ch_range * ch_range;
        unsigned int ret = rexpr_check_status_UNSUCCESS;
        int i;
        
        ro = parent->child;
        while(ro != NULL){
                if(start_S > end){
                        ret = rexpr_check_status_END_OF_LINE;
                        goto break_while;
                }
                switch(ro->type){
                        case rexpr_object_type_STAR:
                                ret = check_str_rexpr_object_STAR(ro, str, &start_S, end);
                                if(ret != rexpr_check_status_SUCCESS)
                                        goto break_while;
                                break;
                        case rexpr_object_type_PLUS:
                                ret = check_str_rexpr_object_PLUS(ro, str, &start_S, end);
                                if(ret != rexpr_check_status_SUCCESS)
                                        goto break_while;
                                break;
                        case rexpr_object_type_ROUND_BRACKETS_OPEN:
                                ret = check_str_rexpr_object_ROUND_BRACKETS_OPEN(ro, str, &start_S, end);
                                if(ret != rexpr_check_status_SUCCESS)
                                        goto break_while;
                                break;
                        case rexpr_object_type_SQUARE_BRACKETS_OPEN:
                                ch_range = ro->data.ch_range;
                                while(ch_range != NULL){
                                        if(ch_range->r == '\0'){
                                                if(ch_range->l == str[start_S]){
                                                        start_S += 1;
                                                        ret = rexpr_check_status_SUCCESS;
                                                        break;
                                                }
                                        } else {
                                                if(ch_range->l <= str[start_S] 
                                                        && ch_range->r >= str[start_S]){
                                                        start_S += 1;
                                                        ret = rexpr_check_status_SUCCESS;
                                                        break;
                                                }
                                        }
                                        ch_range = ch_range->next;
                                }
                                if(ch_range == NULL){
                                        ret = rexpr_check_status_UNSUCCESS;
                                        goto break_while;
                                }
                                break;
                        case rexpr_object_type_STRING:
                                for(i = 0; i < ro->data.str.len; i++){
                                        if(start_S > end){
                                                ret = rexpr_check_status_END_OF_LINE;
                                                goto break_while;
                                        }
                                        if(ro->data.str.str[i] != str[start_S]){
                                                ret = rexpr_check_status_UNSUCCESS;
                                                goto break_while;
                                        }
                                        start_S += 1;
                                }
                                ret = rexpr_check_status_SUCCESS;
                                break;
                        case rexpr_object_type_DOT:
                                start_S += 1;
                                ret = rexpr_check_status_SUCCESS;
                                break;
                        default:
                                PINF("unexpected rexpr_object type");
                                return rexpr_check_status_UNSUCCESS;
                }
                ro = ro->next;
        }
        break_while:
        if(ret == rexpr_check_status_SUCCESS)
                *start = start_S;
        
        return ret;
}

static int check_str_rexpr_object(rexpr_object * parent, const char * str, ssize_t start, ssize_t * end)
{
        /*
                Функция проверяет совпадение строки с регулярным выражением
                Если совпадения нет, значение end не изменяется и возвращается rexpr_check_status_UNSUCCESS
                Если есть совпадение, то в end записывается последний символ совпавшей подстроки и возвращается rexpr_check_status_SUCCESS
                Если при проверке закончилась строка, то возвращается rexpr_check_status_END_OF_LINE
                Функция не ищет подстроку в строке, а только проверяет совпадение, начиная с первого символа
        */
        if(parent == NULL){
                PINF("parent is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(parent->child == NULL){
                PINF("child is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(str == NULL){
                PINF("str is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(parent->type != rexpr_object_type_start_main){
                PINF("parent not rexpr_object_type_start_main");
                return rexpr_check_status_UNSUCCESS;
        }
        ssize_t start_S = start;
        rexpr_object * ro;
        rexpr_object_ch_range * ch_range;
        unsigned int ret = rexpr_check_status_UNSUCCESS;
        int i;
        
        ro = parent->child;
        while(ro != NULL){
                if(start_S > *end){
                        ret = rexpr_check_status_END_OF_LINE;
                        goto break_while;
                }
                switch(ro->type){
                        case rexpr_object_type_STAR:
                                ret = check_str_rexpr_object_STAR(ro, str, &start_S, *end);
                                if(ret != rexpr_check_status_SUCCESS)
                                        goto break_while;
                                break;
                        case rexpr_object_type_PLUS:
                                ret = check_str_rexpr_object_PLUS(ro, str, &start_S, *end);
                                if(ret != rexpr_check_status_SUCCESS)
                                        goto break_while;
                                break;
                        case rexpr_object_type_ROUND_BRACKETS_OPEN:
                                ret = check_str_rexpr_object_ROUND_BRACKETS_OPEN(ro, str, &start_S, *end);
                                if(ret != rexpr_check_status_SUCCESS)
                                        goto break_while;
                                break;
                        case rexpr_object_type_SQUARE_BRACKETS_OPEN:
                                ch_range = ro->data.ch_range;
                                while(ch_range != NULL){
                                        ret = rexpr_check_status_UNSUCCESS;
                                        if(ch_range->r == '\0'){
                                                if(ch_range->l == str[start_S]){
                                                        start_S += 1;
                                                        ret = rexpr_check_status_SUCCESS;
                                                        break;
                                                }
                                        } else {
                                                if(ch_range->l <= str[start_S] 
                                                        && ch_range->r >= str[start_S]){
                                                        start_S += 1;
                                                        ret = rexpr_check_status_SUCCESS;
                                                        break;
                                                }
                                        }
                                        ch_range = ch_range->next;
                                }
                                if(ret != rexpr_check_status_SUCCESS)
                                        goto break_while;
                                if(ch_range == NULL){
                                        ret = rexpr_check_status_UNSUCCESS;
                                        goto break_while;
                                }
                                break;
                        case rexpr_object_type_STRING:
                                for(i = 0; i < ro->data.str.len; i++){
                                        if(start_S > *end){
                                                ret = rexpr_check_status_END_OF_LINE;
                                                goto break_while;
                                        }
                                        if(ro->data.str.str[i] != str[start_S]){
                                                ret = rexpr_check_status_UNSUCCESS;
                                                goto break_while;
                                        }
                                        start_S += 1;
                                }
                                ret = rexpr_check_status_SUCCESS;
                                break;
                        case rexpr_object_type_DOT:
                                start_S += 1;
                                ret = rexpr_check_status_SUCCESS;
                                break;
                        default:
                                PINF("unexpected rexpr_object type");
                                return rexpr_check_status_UNSUCCESS;
                }
                ro = ro->next;
        }
        break_while:
        if(start >= start_S)
                return rexpr_check_status_UNSUCCESS;
        if(ret == rexpr_check_status_SUCCESS)
                *end = start_S - 1;
        
        return ret;
}

ssize_t rexpr_find(const char * str, ssize_t str_len, const char * opt, ssize_t opt_len, ssize_t * end_substr)
{
        /*
                str     - строка
                opt     - регулярное выражение
                *end_substr   - последний символ найденой подстроки
                Функция возвращает позицию первого символа найденой подстроки
                Либо -1
        */
        rexpr_object ro_main;
        ssize_t start = 0;
        ssize_t end = opt_len - 1;
        
        ro_main.type = rexpr_object_type_start_main;
        ro_main.next = NULL;
        ro_main.child = NULL;
        
        if(0 != parse_rexpr_object(&ro_main, opt, start, &end))
                return -1;
                
        end = str_len - 1;
        while(start <= end){
                switch(check_str_rexpr_object(&ro_main, str, start, &end)){
                        case rexpr_check_status_SUCCESS:
                                *end_substr = end;
                                return start;
                        default:
                                start += 1;
                }
        }
        
        return -1;
}