#include "rexpr.h"
#include "debug.h"

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
static char rexpr_object_type_second_to_ch[rexpr_object_type_second_end_ch + 1] = {'\\', '^', '|'};
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

static int is_one_byte_ch(char ch)
{
        /*
                Функция возвращает 1, если символ ch однобайтовый
                0, если это чать многобайтового символа
        */
        return (ch >> 7) != 0 ? 0 : 1;
}

static int is_two_byte_ch(const char * str, ssize_t pos, ssize_t len)
{
        /*
                Функция возвращает 1, если символ в str[pos] двубайтовый
                Либо 0
        */
        if(0 == is_one_byte_ch(str[pos])){
                if((pos + 1) <= len)
                        return 1;
        }
        return 0;
}

/*
        Функции парсинга регулярного выражения
*/
static int parse_rexpr_object_create_STRING(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end)
{
        PFUNC_START();
        if(parent == NULL){
                PERR("parent is NULL");
                return -1;
        }
        if(opt == NULL){
                PERR("opt is NULL");
                return -1;
        }
        if(start > *end){
                PERR("start > end");
                return -1;
        }
        ssize_t start_str;
        size_t idx, idx2;
        rexpr_object * ro;
        
        ro = malloc(sizeof(rexpr_object));
        if(ro == NULL){
                PCERR("rexpr_object: malloc()");
                return -1;
        }
        ro->child = NULL;
        ro->next = NULL;
        
        switch(parent->type){
                case rexpr_object_type_STAR:
                case rexpr_object_type_PLUS:
                        switch (is_one_byte_ch(opt[*end]))
                        {
                                case 1:
                                        ro->data.str.str = malloc(sizeof(char));
                                        if(ro->data.str.str == NULL){
                                                PCERR("rexpr_object_str->str: malloc()");
                                                return -1;
                                        }
                                        ro->data.str.len = 1;
                                        ro->data.str.str[0] = opt[*end];
                                        *end -= 1;
                                        break;
                                case 0:
                                        if( (*end - 1) < start){
                                                PERR("start > end");
                                                return -1;
                                        }
                                        ro->data.str.str = malloc(sizeof(char) * 2);
                                        if(ro->data.str.str == NULL){
                                                PCERR("rexpr_object_str->str: malloc()");
                                                return -1;
                                        }
                                        ro->data.str.len = 2;
                                        ro->data.str.str[1] = opt[*end];
                                        ro->data.str.str[0] = opt[*end - 1];
                                        *end -= 2;
                                        break;
                        }
                        
                        break;
                default:
                        start_str = *end + 1;
                        while( (start_str - 1) >= start ){
                                if(rexpr_object_type_to_int(opt[start_str - 1]) != rexpr_object_type_STRING)
                                        break;
                                start_str -= 1;
                        }
                        ro->data.str.len = *end - start_str + 1;
                        ro->data.str.str = malloc(sizeof(char) * ro->data.str.len);
                        if(ro->data.str.str == NULL){
                                PCERR("rexpr_object_str->str: malloc()");
                                return -1;
                        }
                        for(idx = 0, idx2 = start_str; idx2 <= *end; idx2++, idx++){
                                ro->data.str.str[idx] = opt[idx2];
                        }
                        *end = start_str - 1;
        }
        
        ro->type = rexpr_object_type_STRING;
        
        ro->next = parent->child;
        parent->child = ro;
#if DBG_LVL >= 2
        int dbg_i;
        PRINT("%lu - %ld / %ld / ", start, *end, ro->data.str.len);
        for(dbg_i = 0; dbg_i <= ro->data.str.len; dbg_i++)
                PRINT("%c", ro->data.str.str[dbg_i]);
        PRINT("\n");
#endif
        PFUNC_END();
        return 0;
}

static int parse_rexpr_object_create_DOT(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end)
{
        PFUNC_START();
        PRINT("%lu - %ld\n", start, *end);
        if(parent == NULL){
                PERR("parent is NULL");
                return -1;
        }
        if(opt == NULL){
                PERR("opt is NULL");
                return -1;
        }
        if(start > *end){
                PERR("start > end");
                return -1;
        }
        rexpr_object * ro;
        
        ro = malloc(sizeof(rexpr_object));
        if(ro == NULL){
                PCERR("rexpr_object: malloc()");
                return -1;
        }
        ro->child = NULL;
        ro->next = NULL;
        
        ro->type = rexpr_object_type_DOT;
        *end -= 1;
        
        ro->next = parent->child;
        parent->child = ro;
                
        PFUNC_END();
        return 0;
}

static int parse_rexpr_object_create_STAR(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end)
{
        PFUNC_START();
        PRINT("%lu - %ld\n", start, *end);
        if(parent == NULL){
                PERR("parent is NULL");
                return -1;
        }
        if(opt == NULL){
                PERR("opt is NULL");
                return -1;
        }
        if(start > *end){
                PERR("start > end");
                return -1;
        }
        rexpr_object * ro;
        
        ro = malloc(sizeof(rexpr_object));
        if(ro == NULL){
                PCERR("rexpr_object: malloc()");
                return -1;
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
                        
                        PFUNC_END();
                        return parse_rexpr_object(ro, opt, start, end);
                default:
                        PERR("unexpected parent type");
                        return -1;
        }
        
        return -1;
}

static int parse_rexpr_object_create_PLUS(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end)
{
        PFUNC_START();
        PRINT("%lu - %ld\n", start, *end);
        if(parent == NULL){
                PERR("parent is NULL");
                return -1;
        }
        if(opt == NULL){
                PERR("opt is NULL");
                return -1;
        }
        if(start > *end){
                PERR("start > end");
                return -1;
        }
        rexpr_object * ro;
        
        ro = malloc(sizeof(rexpr_object));
        if(ro == NULL){
                PCERR("rexpr_object: malloc()");
                return -1;
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
                        
                        PFUNC_END();
                        return parse_rexpr_object(ro, opt, start, end);
                default:
                        PERR("unexpected parent type");
                        return -1;
        }
        
        
        return -1;
}

static int parse_rexpr_object_create_ROUND_BRACKETS_OPEN(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end)
{
        PFUNC_START();
        PRINT("%lu - %ld\n", start, *end);
        if(parent == NULL){
                PERR("parent is NULL");
                return -1;
        }
        if(opt == NULL){
                PERR("opt is NULL");
                return -1;
        }
        if(start > *end){
                PERR("start > end");
                return -1;
        }
        rexpr_object * ro;
        
        ro = malloc(sizeof(rexpr_object));
        if(ro == NULL){
                PCERR("rexpr_object: malloc()");
                return -1;
        }
        ro->child = NULL;
        ro->next = NULL;
        
        switch(parent->type){
                case rexpr_object_type_start_main:
                case rexpr_object_type_STAR:
                case rexpr_object_type_PLUS:
                case rexpr_object_type_ROUND_BRACKETS_OPEN:
                        ro->type = rexpr_object_type_ROUND_BRACKETS_OPEN;
                        ro->data.type = rexpr_object_type_second_unknown_ch;
                        *end -= 1;
                        
                        while(ro->data.type == rexpr_object_type_second_unknown_ch)
                                if(0 != parse_rexpr_object(ro, opt, start, end))
                                        return -1;
                        ro->next = parent->child;
                        parent->child = ro;
                        
                        PFUNC_END();
                        return 0;
                default:
                        PERR("unexpected parent type");
                        return -1;
        }
        
        return -1;
}

static int parse_rexpr_object_create_ROUND_BRACKETS_CLOSE(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end)
{
        PFUNC_START();
        PRINT("%lu - %ld\n", start, *end);
        if(parent == NULL){
                PERR("parent is NULL");
                return -1;
        }
        if(opt == NULL){
                PERR("opt is NULL");
                return -1;
        }
        if(start > *end){
                PERR("start > end");
                return -1;
        }
        
        switch(parent->type){
                case rexpr_object_type_ROUND_BRACKETS_OPEN:
                        if( (*end - 1) >= start ){
                                if(opt[*end - 1] == rexpr_object_type_second_to_ch[rexpr_object_type_second_NOT]){
                                        parent->data.type = rexpr_object_type_second_NOT;
                                        *end -= 2;
                                        
                                        PFUNC_END();
                                        return 0;
                                }
                                if(opt[*end - 1] == rexpr_object_type_second_to_ch[rexpr_object_type_second_OR]){
                                        parent->data.type = rexpr_object_type_second_OR;
                                        *end -= 2;
                                        
                                        PFUNC_END();
                                        return 0;
                                }
                        }
                        parent->data.type = rexpr_object_type_ROUND_BRACKETS_CLOSE;
                        *end -= 1;
                        
                        PFUNC_END();
                        return 0;
                default:
                        PERR("unexpected parent type");
                        return -1;
        }
        
        return -1;
}

static int parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(rexpr_object * parent, const char * l, ssize_t l_len, const char * r, ssize_t r_len)
{
        PFUNC_START();
        if(parent == NULL){
                PERR("parent is NULL");
                return -1;
        }
        rexpr_object_ch_range * ch_range;
        
        ch_range = malloc(sizeof(rexpr_object_ch_range));
        if(ch_range == NULL){
                PCERR("struct rexpr_object_ch_range: malloc()");
                return -1;
        }
        bzero(ch_range->l, MAX_CH_LEN);
        if(ch_range->l != NULL && l_len > 0)
                memcpy(ch_range->l, l, l_len);
        bzero(ch_range->r, MAX_CH_LEN);
        if(ch_range->r != NULL && r_len > 0)
                memcpy(ch_range->r, r, r_len);
        ch_range->next = NULL;
        if(parent->data.ch_range == NULL){
                parent->data.ch_range = ch_range;
        } else {
                ch_range->next = parent->data.ch_range;
                parent->data.ch_range = ch_range;
        }
        
        PRINT("%c%c - %c%c\n", ch_range->l[0], ch_range->l[1], ch_range->r[0], ch_range->r[1]);
        
        PFUNC_END();
        return 0;
}

static int parse_rexpr_object_create_SQUARE_BRACKETS_OPEN(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end)
{
        PFUNC_START();
        PRINT("%lu - %ld\n", start, *end);
        if(parent == NULL){
                PERR("parent is NULL");
                return -1;
        }
        if(opt == NULL){
                PERR("opt is NULL");
                return -1;
        }
        if(start > *end){
                PERR("start > end");
                return -1;
        }
        rexpr_object * ro;
        
        ro = malloc(sizeof(rexpr_object));
        if(ro == NULL){
                PCERR("rexpr_object: malloc()");
                return -1;
        }
        ro->child = NULL;
        ro->next = NULL;
        char escape_tmp;
        unsigned int istb;
        
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
                                                        if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, opt + *end, 1, NULL, 0))
                                                                return -1;
                                                        *end -= 1;
                                                }
                                        }
                                        *end -= 1;
                                        ro->next = parent->child;
                                        parent->child = ro;
                                        
                                        PFUNC_END();
                                        return 0;
                                }
                                if((*end - 1) >= start){
                                        /* Если '*+' */
                                        if(opt[*end - 1] == rexpr_object_type_second_to_ch[rexpr_object_type_second_ESCAPE]){
                                                /* Если '\+' */
                                                if(rexpr_escape_type_to_int(opt[*end]) != rexpr_escape_type_unknown_ch){
                                                        escape_tmp = rexpr_escape_type_to_ch[rexpr_escape_type_to_int(opt[*end])];
                                                        if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, &escape_tmp, 1, NULL, 0))
                                                                return -1;
                                                        *end -= 2;
                                                        continue;
                                                }
                                        }
                                }
                                istb = 0;
                                if(0 == is_one_byte_ch(opt[*end])){
                                        /*Если текущий символ двубайтовый*/
                                        if((*end - 1) >= start){
                                                *end -= 1;
                                                istb = 1;
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
                                                                                return -1;
                                                                        if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, opt + (*end - 2), 
                                                                                istb == 1 ? 2 : 1, opt + *end, 1))
                                                                                return -1;
                                                                        *end -= 4;
                                                                        ro->next = parent->child;
                                                                        parent->child = ro;
                                                                        
                                                                        PFUNC_END();
                                                                        return 0;
                                                                }
                                                        }
                                                        if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, opt + *end, istb == 1 ? 2 : 1, NULL, 0))
                                                                return -1;
                                                        if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, opt + (*end - 1), 1, NULL, 0))
                                                                return -1;
                                                        *end -= 3;
                                                        ro->next = parent->child;
                                                        parent->child = ro;
                                                        
                                                        PFUNC_END();
                                                        return 0;
                                                }
                                                if(0 == is_one_byte_ch(opt[*end - 2])){
                                                        /*Если символ за '-' тоже двубайтовый*/
                                                        if((*end - 3) >= start){
                                                                istb = 2;
                                                        }
                                                }
                                                if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(
                                                        ro, 
                                                        istb > 0 ? opt + (*end - 3) : opt + (*end - 2), 
                                                        istb > 0 ? 2 : 1, 
                                                        opt + *end, 
                                                        istb == 2 ? 2 : 1))
                                                        return -1;
                                                if(istb > 0)
                                                        *end -= 4;
                                                else
                                                        *end -= 3;
                                                continue;
                                        }
                                }
                                if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, opt + *end, istb == 1 ? 2 : 1, NULL, 0))
                                        return -1;
                                *end -= 1;
                        }
                        break;
                default:
                        PERR("unexpected parent type");
                        return -1;
        }
        
        return -1;
}

int parse_rexpr_object(rexpr_object * parent, const char * opt, ssize_t start, ssize_t * end)
{
        /*
                Функция парсит регулярное выражение, создает его представление в структурах
        */
        PFUNC_START();
        if(parent == NULL){
                PERR("parent is NULL");
                return -1;
        }
        if(opt == NULL){
                PERR("opt is NULL");
                return -1;
        }
        switch(parent->type){
                case rexpr_object_type_start_main:
                case rexpr_object_type_STAR:
                case rexpr_object_type_PLUS:
                case rexpr_object_type_ROUND_BRACKETS_OPEN:
                case rexpr_object_type_SQUARE_BRACKETS_OPEN:
                        if(start > *end){
                                PERR("start > end");
                                return -1;
                        }
                        break;
        }
        
        while(1){
                PRINT("parse_rexpr_object: parent: %c\n", rexpr_object_type_to_ch[parent->type]);
                switch(rexpr_object_type_to_int(opt[*end])){
                        case rexpr_object_type_DOT:
                                if(0 != parse_rexpr_object_create_DOT(parent, opt, start, end))
                                                return -1;
                                break;
                        case rexpr_object_type_STAR:
                                if(0 != parse_rexpr_object_create_STAR(parent, opt, start, end))
                                                return -1;
                                break;
                        case rexpr_object_type_PLUS:
                                if(0 != parse_rexpr_object_create_PLUS(parent, opt, start, end))
                                                return -1;
                                break;
                        case rexpr_object_type_ROUND_BRACKETS_OPEN:
                                if(0 != parse_rexpr_object_create_ROUND_BRACKETS_OPEN(parent, opt, start, end))
                                                return -1;
                                break;
                        case rexpr_object_type_ROUND_BRACKETS_CLOSE:
                                if(0 != parse_rexpr_object_create_ROUND_BRACKETS_CLOSE(parent, opt, start, end))
                                                return -1;
                                break;
                        case rexpr_object_type_SQUARE_BRACKETS_OPEN:
                                if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN(parent, opt, start, end))
                                                return -1;
                                break;
                        //case rexpr_object_type_SQUARE_BRACKETS_CLOSE:
                        case rexpr_object_type_STRING:
                                if(0 != parse_rexpr_object_create_STRING(parent, opt, start, end))
                                        return -1;
                                break;
                        default:
                                PERR("unexpected rexpr_object type");
                                return -1;
                }
                if(parent->type == rexpr_object_type_start_main){
                        if(*end < start){
                                PFUNC_END();
                                return 0;
                        }
                } else
                        break;
        }
        
        PFUNC_END();
        return 0;
}

/*
        Функции поиска
*/
static int check_str_rexpr_object_ROUND_BRACKETS_OPEN(rexpr_object * parent, char * str, ssize_t * start, ssize_t * end,
                void (* get_next_str)(char ** str, ssize_t * start, ssize_t * end, void * data),
                void * data);
static int check_str_rexpr_object_STAR(rexpr_object * parent, char * str, ssize_t * start, ssize_t * end,
                void (* get_next_str)(char ** str, ssize_t * start, ssize_t * end, void * data),
                void * data)
{
        PFUNC_START();
        if(parent == NULL){
                PERR("parent is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(parent->child == NULL){
                PERR("child is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(str == NULL){
                PERR("str is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        ssize_t start_S = *start;
        ssize_t start_S1;
        rexpr_object * ro = parent->child;
        rexpr_object_ch_range * ch_range;
        int i;
        unsigned int istb;
        
        while(1)
        switch(ro->type){
                case rexpr_object_type_ROUND_BRACKETS_OPEN:
                        start_S1 = start_S;
                        if(rexpr_check_status_SUCCESS != check_str_rexpr_object_ROUND_BRACKETS_OPEN(ro, str, &start_S, end, get_next_str, data))
                                goto break_while;
                        if(ro->data.type == rexpr_object_type_second_NOT && start_S1 == start_S){
                                start_S += 1;
                        }
                        break;
                case rexpr_object_type_SQUARE_BRACKETS_OPEN:
                        if(start_S > *end){
                                char * tmp_str = str;
                                if(get_next_str != NULL){
                                        get_next_str(&str, &start_S, end, data);
                                } else
                                        str = NULL;
                                
                                if(str == NULL){
                                        str = tmp_str;
                                        goto break_while;
                                }
                        }
                        ch_range = ro->data.ch_range;
                        while(ch_range != NULL){
                                istb = is_two_byte_ch(str, start_S, *end);
                                if(ch_range->r[0] == '\0'){
                                        if(istb != 0){
                                                if(0 == memcmp(ch_range->l, str + start_S, 2)){
                                                        start_S += 2;
                                                        break;
                                                }
                                        } else {
                                                if(0 == memcmp(ch_range->l, str + start_S, 1)){
                                                        start_S += 1;
                                                        break;
                                                }
                                        }
                                } else {
                                        if(istb != 0){
                                                if(0 <= memcmp(str + start_S, ch_range->l, 2) && 0 >= memcmp(str + start_S, ch_range->r, 2)){
                                                        start_S += 2;
                                                        break;
                                                }
                                        } else {
                                                if(0 <= memcmp(str + start_S, ch_range->l, 1) && 0 >= memcmp(str + start_S, ch_range->r, 1)){
                                                        start_S += 1;
                                                        break;
                                                }
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
                                if(start_S1 > *end){
                                        char * tmp_str = str;
                                        if(get_next_str != NULL){
                                                get_next_str(&str, &start_S1, end, data);
                                        } else
                                                str = NULL;
                                        
                                        if(str == NULL){
                                                str = tmp_str;
                                                goto break_while;
                                        }
                                }
                                if(ro->data.str.str[i] != str[start_S1])
                                        goto break_while;
                                start_S1 += 1;
                        }
                        start_S = start_S1;
                        break;
                case rexpr_object_type_DOT:
                        if(start_S > *end){
                                char * tmp_str = str;
                                if(get_next_str != NULL){
                                        get_next_str(&str, &start_S, end, data);
                                } else
                                        str = NULL;
                                
                                if(str == NULL){
                                        str = tmp_str;
                                        goto break_while;
                                }
                        }
                        if(1 == is_two_byte_ch(str, start_S, *end))
                                start_S += 2;
                        else
                                start_S += 1;
                        break;
                default:
                        PERR("unexpected rexpr_object type");
                        return rexpr_check_status_UNSUCCESS;
        }
        break_while:
        *start = start_S;
        
        PFUNC_END();
        return rexpr_check_status_SUCCESS;
}

static int check_str_rexpr_object_PLUS(rexpr_object * parent, char * str, ssize_t * start, ssize_t * end,
                void (* get_next_str)(char ** str, ssize_t * start, ssize_t * end, void * data),
                void * data)
{
        PFUNC_START();
        if(parent == NULL){
                PERR("parent is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(parent->child == NULL){
                PERR("child is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(str == NULL){
                PERR("str is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        ssize_t start_S = *start;
        ssize_t start_S1;
        rexpr_object * ro = parent->child;
        rexpr_object_ch_range * ch_range;
        int i;
        int count_SUCCESS = 0;
        unsigned istb;
        
        while(1){
                switch(ro->type){
                        case rexpr_object_type_ROUND_BRACKETS_OPEN:
                                start_S1 = start_S;
                                if(rexpr_check_status_SUCCESS != check_str_rexpr_object_ROUND_BRACKETS_OPEN(ro, str, &start_S, end, get_next_str, data))
                                        goto break_while;
                                if(ro->data.type == rexpr_object_type_second_NOT && start_S1 == start_S){
                                        start_S += 1;
                                }
                                break;
                        case rexpr_object_type_SQUARE_BRACKETS_OPEN:
                                if(start_S > *end){
                                        char * tmp_str = str;
                                        if(get_next_str != NULL){
                                                get_next_str(&str, &start_S, end, data);
                                        } else
                                                str = NULL;
                                        
                                        if(str == NULL){
                                                str = tmp_str;
                                                goto break_while;
                                        }
                                }
                                ch_range = ro->data.ch_range;
                                while(ch_range != NULL){
                                        istb = is_two_byte_ch(str, start_S, *end);
                                        if(ch_range->r[0] == '\0'){
                                                if(istb != 0){
                                                        if(0 == memcmp(ch_range->l, str + start_S, 2)){
                                                                start_S += 2;
                                                                break;
                                                        }
                                                } else {
                                                        if(0 == memcmp(ch_range->l, str + start_S, 1)){
                                                                start_S += 1;
                                                                break;
                                                        }
                                                }
                                        } else {
                                                if(istb != 0){
                                                        if(0 <= memcmp(str + start_S, ch_range->l, 2) && 0 >= memcmp(str + start_S, ch_range->r, 2)){
                                                                start_S += 2;
                                                                break;
                                                        }
                                                } else {
                                                        if(0 <= memcmp(str + start_S, ch_range->l, 1) && 0 >= memcmp(str + start_S, ch_range->r, 1)){
                                                                start_S += 1;
                                                                break;
                                                        }
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
                                        if(start_S1 > *end){
                                                char * tmp_str = str;
                                                if(get_next_str != NULL){
                                                        get_next_str(&str, &start_S1, end, data);
                                                } else
                                                        str = NULL;
                                                
                                                if(str == NULL){
                                                        str = tmp_str;
                                                        goto break_while;
                                                }
                                        }
                                        if(ro->data.str.str[i] != str[start_S1])
                                                goto break_while;
                                        start_S1 += 1;
                                }
                                start_S = start_S1;
                                break;
                        case rexpr_object_type_DOT:
                                if(start_S > *end){
                                        char * tmp_str = str;
                                        if(get_next_str != NULL){
                                                get_next_str(&str, &start_S, end, data);
                                        } else
                                                str = NULL;
                                        
                                        if(str == NULL){
                                                str = tmp_str;
                                                goto break_while;
                                        }
                                }
                                if(1 == is_two_byte_ch(str, start_S, *end))
                                        start_S += 2;
                                else
                                        start_S += 1;
                                break;
                        default:
                                PERR("unexpected rexpr_object type");
                                return rexpr_check_status_UNSUCCESS;
                }
                count_SUCCESS += 1;
        }
        break_while:
        PFUNC_END();
        if(count_SUCCESS < 1){
                return rexpr_check_status_UNSUCCESS;
        } else {
                *start = start_S;
                return rexpr_check_status_SUCCESS;
        }
        
        return rexpr_check_status_UNSUCCESS;
}

static int check_str_rexpr_object_ROUND_BRACKETS_OPEN(rexpr_object * parent, char * str, ssize_t * start, ssize_t * end,
                void (* get_next_str)(char ** str, ssize_t * start, ssize_t * end, void * data),
                void * data)
{
        PFUNC_START();
        if(parent == NULL){
                PERR("parent is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(parent->child == NULL){
                PERR("child is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(str == NULL){
                PERR("str is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        ssize_t start_S = *start;
        ssize_t start_S1;
        rexpr_object * ro;
        rexpr_object_ch_range * ch_range;
        unsigned int ret = rexpr_check_status_UNSUCCESS;
        int i;
        unsigned int istb;
        
        ro = parent->child;
        while(ro != NULL){
                if(start_S > *end){
                        if(get_next_str != NULL){
                                get_next_str(&str, &start_S, end, data);
                        } else
                                str = NULL;
                        
                        if(str == NULL){
                                ret = rexpr_check_status_END_OF_LINE;
                                goto break_while;
                        } else
                                continue;
                }
                switch(ro->type){
                        case rexpr_object_type_STAR:
                                ret = check_str_rexpr_object_STAR(ro, str, &start_S, end, get_next_str, data);
                                if(parent->data.type == rexpr_object_type_second_NOT){
                                        if(ret == rexpr_check_status_SUCCESS){
                                                ret = rexpr_check_status_UNSUCCESS;
                                        } else if(ret == rexpr_check_status_UNSUCCESS){
                                                ret = rexpr_check_status_SUCCESS;
                                        }
                                }
                                if(ret == rexpr_check_status_SUCCESS && parent->data.type == rexpr_object_type_second_OR)
                                        goto break_while;
                                if(ret != rexpr_check_status_SUCCESS && parent->data.type != rexpr_object_type_second_OR)
                                        goto break_while;
                                break;
                        case rexpr_object_type_PLUS:
                                ret = check_str_rexpr_object_PLUS(ro, str, &start_S, end, get_next_str, data);
                                if(parent->data.type == rexpr_object_type_second_NOT){
                                        if(ret == rexpr_check_status_SUCCESS){
                                                ret = rexpr_check_status_UNSUCCESS;
                                        } else if(ret == rexpr_check_status_UNSUCCESS){
                                                ret = rexpr_check_status_SUCCESS;
                                        }
                                }
                                if(ret == rexpr_check_status_SUCCESS && parent->data.type == rexpr_object_type_second_OR)
                                        goto break_while;
                                if(ret != rexpr_check_status_SUCCESS && parent->data.type != rexpr_object_type_second_OR)
                                        goto break_while;
                                break;
                        case rexpr_object_type_ROUND_BRACKETS_OPEN:
                                start_S1 = start_S;
                                ret = check_str_rexpr_object_ROUND_BRACKETS_OPEN(ro, str, &start_S, end, get_next_str, data);
                                if(ret == rexpr_check_status_SUCCESS && ro->data.type == rexpr_object_type_second_NOT && start_S1 == start_S){
                                        start_S += 1;
                                }
                                if(parent->data.type == rexpr_object_type_second_NOT){
                                        if(ret == rexpr_check_status_SUCCESS){
                                                ret = rexpr_check_status_UNSUCCESS;
                                        } else if(ret == rexpr_check_status_UNSUCCESS){
                                                ret = rexpr_check_status_SUCCESS;
                                        }
                                }
                                if(ret == rexpr_check_status_SUCCESS && parent->data.type == rexpr_object_type_second_OR)
                                        goto break_while;
                                if(ret != rexpr_check_status_SUCCESS && parent->data.type != rexpr_object_type_second_OR)
                                        goto break_while;
                                break;
                        case rexpr_object_type_SQUARE_BRACKETS_OPEN:
                                ch_range = ro->data.ch_range;
                                while(ch_range != NULL){
                                        istb = is_two_byte_ch(str, start_S, *end);
                                        if(ch_range->r[0] == '\0'){
                                                if(istb != 0){
                                                        if(0 == memcmp(ch_range->l, str + start_S, 2)){
                                                                ret = rexpr_check_status_SUCCESS;
                                                                start_S += 2;
                                                                break;
                                                        }
                                                } else {
                                                        if(0 == memcmp(ch_range->l, str + start_S, 1)){
                                                                ret = rexpr_check_status_SUCCESS;
                                                                start_S += 1;
                                                                break;
                                                        }
                                                }
                                        } else {
                                                if(istb != 0){
                                                        if(0 <= memcmp(str + start_S, ch_range->l, 2) && 0 >= memcmp(str + start_S, ch_range->r, 2)){
                                                                ret = rexpr_check_status_SUCCESS;
                                                                start_S += 2;
                                                                break;
                                                        }
                                                } else {
                                                        if(0 <= memcmp(str + start_S, ch_range->l, 1) && 0 >= memcmp(str + start_S, ch_range->r, 1)){
                                                                ret = rexpr_check_status_SUCCESS;
                                                                start_S += 1;
                                                                break;
                                                        }
                                                }
                                        }
                                        ch_range = ch_range->next;
                                }
                                if(ch_range == NULL)
                                        ret = rexpr_check_status_UNSUCCESS;
                                if(parent->data.type == rexpr_object_type_second_NOT){
                                        if(ret == rexpr_check_status_SUCCESS){
                                                ret = rexpr_check_status_UNSUCCESS;
                                        } else if(ret == rexpr_check_status_UNSUCCESS){
                                                ret = rexpr_check_status_SUCCESS;
                                        }
                                }
                                if(ret == rexpr_check_status_SUCCESS && parent->data.type == rexpr_object_type_second_OR)
                                        goto break_while;
                                if(ret != rexpr_check_status_SUCCESS && parent->data.type != rexpr_object_type_second_OR)
                                        goto break_while;
                                break;
                        case rexpr_object_type_STRING:
                                for(i = 0; i < ro->data.str.len; i++){
                                        if(start_S > *end){
                                                if(get_next_str != NULL){
                                                        get_next_str(&str, &start_S, end, data);
                                                } else
                                                        str = NULL;
                                                
                                                if(str == NULL){
                                                        ret = rexpr_check_status_END_OF_LINE;
                                                        goto break_while;
                                                }
                                        }
                                        if(ro->data.str.str[i] != str[start_S]){
                                                ret = rexpr_check_status_UNSUCCESS;
                                                break;
                                        }
                                        ret = rexpr_check_status_SUCCESS;
                                        start_S += 1;
                                }
                                if(parent->data.type == rexpr_object_type_second_NOT){
                                        if(ret == rexpr_check_status_SUCCESS){
                                                ret = rexpr_check_status_UNSUCCESS;
                                        } else if(ret == rexpr_check_status_UNSUCCESS){
                                                ret = rexpr_check_status_SUCCESS;
                                        }
                                }
                                if(ret == rexpr_check_status_SUCCESS && parent->data.type == rexpr_object_type_second_OR)
                                        goto break_while;
                                if(ret != rexpr_check_status_SUCCESS && parent->data.type != rexpr_object_type_second_OR)
                                        goto break_while;
                                break;
                        case rexpr_object_type_DOT:
                                ret = rexpr_check_status_SUCCESS;
                                if(1 == is_two_byte_ch(str, start_S, *end))
                                        start_S += 2;
                                else
                                        start_S += 1;
                                if(parent->data.type == rexpr_object_type_second_NOT){
                                        if(ret == rexpr_check_status_SUCCESS){
                                                ret = rexpr_check_status_UNSUCCESS;
                                        } else if(ret == rexpr_check_status_UNSUCCESS){
                                                ret = rexpr_check_status_SUCCESS;
                                        }
                                }
                                if(ret == rexpr_check_status_SUCCESS && parent->data.type == rexpr_object_type_second_OR)
                                        goto break_while;
                                if(ret != rexpr_check_status_SUCCESS && parent->data.type != rexpr_object_type_second_OR)
                                        goto break_while;
                                break;
                        default:
                                PERR("unexpected rexpr_object type");
                                return rexpr_check_status_UNSUCCESS;
                }
                ro = ro->next;
        }
        break_while:
        if(ret == rexpr_check_status_SUCCESS)
                *start = start_S;
        
        PFUNC_END();
        return ret;
}

int check_str_rexpr_object(rexpr_object * parent, char * str, ssize_t start, ssize_t * end,
                void (* get_next_str)(char ** str, ssize_t * start, ssize_t * end, void * data),
                void * data)
{
        /*
                Функция проверяет совпадение строки с регулярным выражением
                Если совпадения нет, возвращается rexpr_check_status_UNSUCCESS
                Если есть совпадение, то в end записывается последний символ совпавшей подстроки и возвращается rexpr_check_status_SUCCESS
                Если при проверке закончилась строка, то возвращается rexpr_check_status_END_OF_LINE
                В случае неудачи, *end останется в прежнем значении
                Функция НЕ ИЩЕТ подстроку в строке, а только проверяет совпадение, начиная с первого символа
        */
        PFUNC_START();
        if(parent == NULL){
                PERR("parent is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(parent->child == NULL){
                PERR("child is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(str == NULL){
                PERR("str is NULL");
                return rexpr_check_status_UNSUCCESS;
        }
        if(parent->type != rexpr_object_type_start_main){
                PERR("parent not rexpr_object_type_start_main");
                return rexpr_check_status_UNSUCCESS;
        }
        ssize_t start_S = start;
        ssize_t start_S1;
        rexpr_object * ro;
        rexpr_object_ch_range * ch_range;
        unsigned int ret = rexpr_check_status_UNSUCCESS;
        int i;
        unsigned int istb;
        
        ro = parent->child;
        while(ro != NULL){
                if(start_S > *end){
                        if(get_next_str != NULL){
                                get_next_str(&str, &start_S, end, data);
                        } else
                                str = NULL;
                        
                        if(str == NULL){
                                ret = rexpr_check_status_END_OF_LINE;
                                goto break_while;
                        } else
                                continue;
                }
                switch(ro->type){
                        case rexpr_object_type_STAR:
                                ret = check_str_rexpr_object_STAR(ro, str, &start_S, end, get_next_str, data);
                                if(ret != rexpr_check_status_SUCCESS)
                                        goto break_while;
                                break;
                        case rexpr_object_type_PLUS:
                                ret = check_str_rexpr_object_PLUS(ro, str, &start_S, end, get_next_str, data);
                                if(ret != rexpr_check_status_SUCCESS)
                                        goto break_while;
                                break;
                        case rexpr_object_type_ROUND_BRACKETS_OPEN:
                                start_S1 = start_S;
                                ret = check_str_rexpr_object_ROUND_BRACKETS_OPEN(ro, str, &start_S, end, get_next_str, data);
                                if(ret != rexpr_check_status_SUCCESS)
                                        goto break_while;
                                if(ro->data.type == rexpr_object_type_second_NOT && start_S1 == start_S){
                                        start_S += 1;
                                }
                                break;
                        case rexpr_object_type_SQUARE_BRACKETS_OPEN:
                                ch_range = ro->data.ch_range;
                                while(ch_range != NULL){
                                        istb = is_two_byte_ch(str, start_S, *end);
                                        if(ch_range->r[0] == '\0'){
                                                if(istb != 0){
                                                        if(0 == memcmp(ch_range->l, str + start_S, 2)){
                                                                ret = rexpr_check_status_SUCCESS;
                                                                start_S += 2;
                                                                break;
                                                        }
                                                } else {
                                                        if(0 == memcmp(ch_range->l, str + start_S, 1)){
                                                                ret = rexpr_check_status_SUCCESS;
                                                                start_S += 1;
                                                                break;
                                                        }
                                                }
                                        } else {
                                                if(istb != 0){
                                                        if(0 <= memcmp(str + start_S, ch_range->l, 2) && 0 >= memcmp(str + start_S, ch_range->r, 2)){
                                                                ret = rexpr_check_status_SUCCESS;
                                                                start_S += 2;
                                                                break;
                                                        }
                                                } else {
                                                        if(0 <= memcmp(str + start_S, ch_range->l, 1) && 0 >= memcmp(str + start_S, ch_range->r, 1)){
                                                                ret = rexpr_check_status_SUCCESS;
                                                                start_S += 1;
                                                                break;
                                                        }
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
                                start_S1 = start_S;
                                for(i = 0; i < ro->data.str.len; i++){
                                        if(start_S1 > *end){
                                                if(get_next_str != NULL){
                                                        get_next_str(&str, &start_S1, end, data);
                                                } else
                                                        str = NULL;
                                                
                                                if(str == NULL){
                                                        ret = rexpr_check_status_END_OF_LINE;
                                                        goto break_while;
                                                }
                                        }
                                        if(ro->data.str.str[i] != str[start_S1]){
                                                ret = rexpr_check_status_UNSUCCESS;
                                                goto break_while;
                                        }
                                        start_S1 += 1;
                                }
                                start_S = start_S1;
                                ret = rexpr_check_status_SUCCESS;
                                break;
                        case rexpr_object_type_DOT:
                                ret = rexpr_check_status_SUCCESS;
                                if(1 == is_two_byte_ch(str, start_S, *end))
                                        start_S += 2;
                                else
                                        start_S += 1;
                                break;
                        default:
                                PERR("unexpected rexpr_object type");
                                return rexpr_check_status_UNSUCCESS;
                }
                ro = ro->next;
        }
        break_while:
        switch(ret){
                case rexpr_check_status_SUCCESS:
                        *end = start_S - 1;
                        break;
        }
        
        PFUNC_END();
        return ret;
}

void free_rexpr_objects(rexpr_object * parent)
{
        PFUNC_START();
        if(parent == NULL)
                return;
        if(parent->child == NULL)
                return;
                
        rexpr_object * child, * c_tmp;
        struct rexpr_object_ch_range * range, * r_tmp;
        
        switch(parent->type){
                case rexpr_object_type_DOT:
                        PFUNC_END();
                        return;
                case rexpr_object_type_start_main:
                case rexpr_object_type_PLUS:
                case rexpr_object_type_STAR:
                case rexpr_object_type_ROUND_BRACKETS_OPEN:
                        child = parent->child;
                        while(child != NULL){
                                free_rexpr_objects(child);
                                c_tmp = child;
                                child = child->next;
                                free(c_tmp);
                        }
                        PFUNC_END();
                        return;
                case rexpr_object_type_SQUARE_BRACKETS_OPEN:
                        range = parent->data.ch_range;
                        while(range != NULL){
                                r_tmp = range;
                                range = range->next;
                                free(r_tmp);
                        }
                        PFUNC_END();
                        return;
                case rexpr_object_type_STRING:
                        free(parent->data.str.str);
                        PFUNC_END();
                        return;
        }
        PFUNC_END();
}