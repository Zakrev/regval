#define DBG_LVL 1
#include "../debug.h"

#include "../encoding/utf_8.h"
#define _REXPR_TYPES_STATIC_FUNC_
#include "types.h"
#include "rexpr.h"

/*
	Функции парсинга регулярного выражения
*/
static char parse_rexpr_object_create_STRING(rexpr_object * parent, rexpr_object_data * data)
{
	PFUNC_START();
	PRINT("%lld / %lld\n", (long long)data->start, (long long)data->end);
	if(parent == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(data->start > data->end){
		PERR("unexpected start / end: %lld / %lld", (long long)data->start, (long long)data->end);
		return -1;
	}
	bytes_t tmp_end;
	rexpr_object * ro;
	unsigned char bytes;

	ro = malloc(sizeof(rexpr_object));
	if(ro == NULL){
		PCERR("ptr is NULL");
		return -1;
	}
	ro->child = NULL;
	ro->next = NULL;

	switch(parent->type){
		case rexpr_object_type_STAR:
		case rexpr_object_type_PLUS:
			tmp_end = data->end;
			while(1){
				bytes = get_utf8_letter_size(data->str[tmp_end]);
				if(bytes == 0){
					/*Возможно это часть многобайтового символа*/
					tmp_end -= 1;
					if(data->end < data->start){
						PERR("unexpected start / end: %lld / %lld", (long long)data->start, (long long)data->end);
						return -1;
					}
					continue;
				}
				break;
			}
			data->end = tmp_end;
			ro->data.str.str = malloc(sizeof(uchar_t) * bytes);
			if(ro->data.str.str == NULL){
				PCERR("ptr is NULL");
				return -1;
			}
			ro->data.str.len = bytes;
			memcpy(ro->data.str.str, data->str + data->end, bytes);
			data->end -= 1;
			break;
		default:
			tmp_end = data->end;
			while(tmp_end >= data->start){
				if(get_utf8_letter_size(data->str[tmp_end]))
					if(rexpr_object_type_to_int(data->str[tmp_end]) != rexpr_object_type_STRING){
						break;
				}
				tmp_end -= 1;
			}
			tmp_end += 1;
			ro->data.str.len = data->end - tmp_end + 1;
			ro->data.str.str = malloc(sizeof(uchar_t) * ro->data.str.len);
			if(ro->data.str.str == NULL){
				PCERR("ptr is NULL");
				return -1;
			}
			memcpy(ro->data.str.str, data->str + tmp_end, ro->data.str.len);
			data->end = tmp_end - 1;
	}

	ro->type = rexpr_object_type_STRING;

	ro->next = parent->child;
	parent->child = ro;
#if DBG_LVL >= 2
	PRINT("%lld / %lld: %lld '", (long long)data->start, (long long)data->end, (long long)ro->data.str.len);
	PRINT2(ro->data.str.str, ro->data.str.len);
	PRINT("'\n");
#endif
	PFUNC_END();
	return 0;
}

static char parse_rexpr_object_create_DOT(rexpr_object * parent, rexpr_object_data * data)
{
	PFUNC_START();
	PRINT("%lld / %lld\n", (long long)data->start, (long long)data->end);
	if(parent == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(data->start > data->end){
		PERR("unexpected start / end: %lld / %lld", (long long)data->start, (long long)data->end);
		return -1;
	}
	rexpr_object * ro;

	ro = malloc(sizeof(rexpr_object));
	if(ro == NULL){
		PCERR("ptr is NULL)");
		return -1;
	}
	ro->child = NULL;
	ro->next = NULL;
	
	ro->type = rexpr_object_type_DOT;
	data->end -= 1;
	
	ro->next = parent->child;
	parent->child = ro;

	PFUNC_END();
	return 0;
}

static char parse_rexpr_object(rexpr_object * parent, rexpr_object_data * data);
static char parse_rexpr_object_create_STAR(rexpr_object * parent, rexpr_object_data * data)
{
	PFUNC_START();
	PRINT("%lld / %lld\n", (long long)data->start, (long long)data->end);
	if(parent == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(data->start > data->end){
		PERR("unexpected start / end: %lld / %lld", (long long)data->start, (long long)data->end);
		return -1;
	}
	rexpr_object * ro;
	
	ro = malloc(sizeof(rexpr_object));
	if(ro == NULL){
		PCERR("ptr is NULL");
		return -1;
	}
	ro->child = NULL;
	ro->next = NULL;

	switch(parent->type){
		case rexpr_object_type_ROUND_BRACKETS_OPEN_COPY:
		case rexpr_object_type_ROUND_BRACKETS_OPEN:
			ro->type = rexpr_object_type_STAR;
			data->end -= 1;
			ro->next = parent->child;
			parent->child = ro;
			
			PFUNC_END();
			return parse_rexpr_object(ro, data);
		default:
			PERR("unexpected parent type");
			return -1;
	}
	return -1;
}

static char parse_rexpr_object_create_PLUS(rexpr_object * parent, rexpr_object_data * data)
{
	PFUNC_START();
	PRINT("%lld / %lld\n", (long long)data->start, (long long)data->end);
	if(parent == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(data->start > data->end){
		PERR("unexpected start / end: %lld / %lld", (long long)data->start, (long long)data->end);
		return -1;
	}
	rexpr_object * ro;

	ro = malloc(sizeof(rexpr_object));
	if(ro == NULL){
		PCERR("ptr is NULL");
		return -1;
	}
	ro->child = NULL;
	ro->next = NULL;

	switch(parent->type){
		case rexpr_object_type_ROUND_BRACKETS_OPEN_COPY:
		case rexpr_object_type_ROUND_BRACKETS_OPEN:
			ro->type = rexpr_object_type_PLUS;
			data->end -= 1;
			ro->next = parent->child;
			parent->child = ro;
			
			PFUNC_END();
			return parse_rexpr_object(ro, data);
		default:
			PERR("unexpected parent type: %u", (unsigned int)parent->type);
			return -1;
	}
	return -1;
}

static char parse_rexpr_object_create_ROUND_BRACKETS_OPEN(rexpr_object * parent, rexpr_object_data * data)
{
	PFUNC_START();
	PRINT("%lld / %lld\n", (long long)data->start, (long long)data->end);
	if(parent == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(data->start > data->end){
		PERR("unexpected start / end: %lld / %lld", (long long)data->start, (long long)data->end);
		return -1;
	}
	rexpr_object * ro;

	ro = malloc(sizeof(rexpr_object));
	if(ro == NULL){
		PCERR("ptr is NULL");
		return -1;
	}
	ro->child = NULL;
	ro->next = NULL;
	
	switch(parent->type){
		case rexpr_object_type_STAR:
		case rexpr_object_type_PLUS:
			ro->type = rexpr_object_type_ROUND_BRACKETS_OPEN;
			ro->s_type = rexpr_object_type_second_unknown_ch;
			data->end -= 1;

			while(ro->s_type == rexpr_object_type_second_unknown_ch)
				if(0 != parse_rexpr_object(ro, data))
					return -1;
			if(ro->s_type == rexpr_object_type_second_NOT || ro->s_type == rexpr_object_type_second_BRACE_OPEN){
				PERR("forbidden parent type");
				return -1;
			}
			ro->next = parent->child;
			parent->child = ro;
			
			PFUNC_END();
			return 0;
		case rexpr_object_type_ROUND_BRACKETS_OPEN_COPY:
		case rexpr_object_type_ROUND_BRACKETS_OPEN:
			ro->type = rexpr_object_type_ROUND_BRACKETS_OPEN;
			ro->s_type = rexpr_object_type_second_unknown_ch;
			data->end -= 1;

			while(ro->s_type == rexpr_object_type_second_unknown_ch)
				if(0 != parse_rexpr_object(ro, data))
					return -1;
			ro->next = parent->child;
			parent->child = ro;
			
			PFUNC_END();
			return 0;
		default:
			PERR("unexpected parent type: %d", (int)parent->type);
			return -1;
	}

	return -1;
}

static char parse_rexpr_object_create_ROUND_BRACKETS_CLOSE(rexpr_object * parent, rexpr_object_data * data)
{
	PFUNC_START();
	PRINT("%lld / %lld\n", (long long)data->start, (long long)data->end);
	if(parent == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(data->start > data->end){
		PERR("unexpected start / end: %lld / %lld", (long long)data->start, (long long)data->end);
		return -1;
	}
	ptree_node * node;
	rexpr_object * quest;
	int ptree_pos;
	bytes_t tmp_end;
	switch(parent->type){
		case rexpr_object_type_ROUND_BRACKETS_OPEN:
			if(parent->s_type != rexpr_object_type_second_unknown_ch){
				PERR("parent type 'rexpr_object_type_second_unknown_ch' is expected: %d", (int)parent->s_type);
				return -1;
			}
			data->end -= 1;
			if(data->start <= data->end && 1 == get_utf8_letter_size(data->str[data->end])){
				switch(rexpr_object_type_second_to_int(data->str[data->end])){
					case rexpr_object_type_second_NOT:
						PRINT("type: rexpr_object_type_second_NOT\n");
						parent->s_type = rexpr_object_type_second_NOT;
						data->end -= 1;
						break;
					case rexpr_object_type_second_OR:
						PRINT("type: rexpr_object_type_second_OR\n");
						parent->s_type = rexpr_object_type_second_OR;
						data->end -= 1;
						break;
					case rexpr_object_type_second_BRACE_OPEN:
						PRINT("type: rexpr_object_type_second_BRACE_OPEN\n");
						parent->s_type = rexpr_object_type_second_BRACE_OPEN;
						parent->data.len_range.l = 0;
						parent->data.len_range.r = 0;
						tmp_end = data->end;
						while(1){
							/*Поиск закрывающей скобки {*/
							tmp_end -= 1;
							if(tmp_end < data->start){
								PERR("unexpected EOL");
								return -1;
							}
							if(data->str[tmp_end] == rexpr_object_type_second_to_ch[rexpr_object_type_second_BRACE_CLOSE])
								break;
						}
						if(2 != sscanf((char *)(data->str + tmp_end), "{%hu,%hu}", &parent->data.len_range.l, &parent->data.len_range.r)){
							PERR("unexpected format for 'rexpr_object_type_second_BRACE_CLOSE'");
							return -1;
						}
						if(parent->data.len_range.l > parent->data.len_range.r){
							PERR("unexpected format for 'rexpr_object_type_second_BRACE_CLOSE'");
							return -1;
						}
						data->end = tmp_end - 1;
						PRINT("repeat range: {%u,%u}\n", (unsigned int)parent->data.len_range.l, (unsigned int)parent->data.len_range.r);
						break;
					case rexpr_object_type_second_ANGLE_BRACKETS_OPEN:
						PRINT("type: rexpr_object_type_second_ANGLE_BRACKETS_OPEN\n");
						parent->s_type = rexpr_object_type_second_ANGLE_BRACKETS_OPEN;
						data->group_count += 1;
						parent->data.group_id = data->group_count;
						tmp_end = data->end;
						while(1){
							/*Поиск закрывающей скобки <*/
							tmp_end -= 1;
							if(tmp_end < data->start){
								PERR("unexpected EOL");
								return -1;
							}
							if(data->str[tmp_end] == rexpr_object_type_second_to_ch[rexpr_object_type_second_ANGLE_BRACKETS_CLOSE])
								break;
						}
#if DBG_LVL >= 2
						PRINT("create name of group: ");
						PRINT2(data->str + (tmp_end + 1), data->end - (tmp_end + 1));
						PRINT("\n");
#endif
						node = get_node_ptree(data->groups, data->str + (tmp_end + 1), data->end - (tmp_end + 1));
						if(node == NULL){
							ptree_pos = insert_data_ptree(data->groups, data->str + (tmp_end + 1), data->end - (tmp_end + 1), (void *)parent);
							if(0 != ptree_pos){
								PERR("zero position is expected: %d", ptree_pos);
								return -1;
							}
						} else {
							/*Замена первого "запроса" на эту группу*/
							quest = (rexpr_object *)node->data[0];
							if(quest->type != rexpr_object_type_null){
								PERR("'rexpr_object_type_null' is expected: %u", (unsigned int)quest->type);
								return -1;
							}
							/*Копирование правил этой группы*/
							quest->type = rexpr_object_type_ROUND_BRACKETS_OPEN_COPY;
							quest->s_type = rexpr_object_type_second_ANGLE_BRACKETS_OPEN;
							quest->child = parent->child;
							quest->data.group_id = parent->data.group_id;
							node->data[0] = (void *)parent;	//замена
							/*Поиск "запросов" на копирование этой группы*/
							for(ptree_pos = 1; ptree_pos < node->data_size; ptree_pos++){
								quest = (rexpr_object *)(node->data[ptree_pos]);
								if(quest->type != rexpr_object_type_null){
									PERR("'rexpr_object_type_null' is expected: %u", (unsigned int)quest->type);
									return -1;
								}
								/*Копирование правил этой группы*/
								quest->type = rexpr_object_type_ROUND_BRACKETS_OPEN_COPY;
								quest->s_type = rexpr_object_type_second_ANGLE_BRACKETS_OPEN;
								quest->child = parent->child;
								quest->data.group_id = parent->data.group_id;
								/*Удаление из списка "запросов"*/
								if(0 != erase_data_from_node_ptree(node, ptree_pos)){
									PERR("fault erase");
									return -1;
								}
							}
						}
						data->end = tmp_end - 1;
						break;
					default:
						parent->s_type = rexpr_object_type_second_ROUND_BRACKETS_CLOSE;
				}
			} else {
				PRINT("type: rexpr_object_type_second_ROUND_BRACKETS_CLOSE\n");
				parent->s_type = rexpr_object_type_second_ROUND_BRACKETS_CLOSE;
			}
			PFUNC_END();
			return 0;
		default:
			PERR("unexpected parent type");
			return -1;
	}
	return -1;
}

static char parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(rexpr_object * parent, uchar_t * l, uchar_t * r, unsigned int bytes)
{
	PFUNC_START();
	if(parent == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(l != NULL && r != NULL){
		if(0 < memcmp(l, r, bytes)){
			PRINT("bad range: ");
			PRINT2((char *)l, bytes);
			PRINT(" / ");
			PRINT2((char *)r, bytes);
			PRINT("\n");
			PERR("expected area from the smallest to the largest");
			return -1;
		}
	}
	rexpr_object_ch_range * ch_range;

	ch_range = malloc(sizeof(rexpr_object_ch_range));
	if(ch_range == NULL){
		PCERR("ptr is NULL");
		return -1;
	}
	ch_range->bytes = bytes;
	bzero(ch_range->l, MAX_CH_LEN);
	memcpy(ch_range->l, l, bytes);
	bzero(ch_range->r, MAX_CH_LEN);
	if(r != NULL)
		memcpy(ch_range->r, r, bytes);
	ch_range->next = NULL;
	if(parent->data.ch_range == NULL){
		parent->data.ch_range = ch_range;
	} else {
		ch_range->next = parent->data.ch_range;
		parent->data.ch_range = ch_range;
	}

	PRINT("new range: '");
	PRINT2((char *)ch_range->l, ch_range->bytes);
	PRINT("' ... '");
	PRINT2((char *)ch_range->r, ch_range->bytes);
	PRINT("'\n");

	PFUNC_END();
	return 0;
}

static char parse_rexpr_object_create_SQUARE_BRACKETS_OPEN(rexpr_object * parent, rexpr_object_data * data)
{
	PFUNC_START();
	PRINT("%lld / %lld\n", (long long)data->start, (long long)data->end);
	if(parent == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(data->start > data->end){
		PERR("unexpected start / end: %lld / %lld", (long long)data->start, (long long)data->end);
		return -1;
	}
	rexpr_object * ro;

	ro = malloc(sizeof(rexpr_object));
	if(ro == NULL){
		PCERR("ptr is NULL");
		return -1;
	}
	ro->child = NULL;
	ro->next = NULL;
	uchar_t esc;
	unsigned char bytes;

	switch(parent->type){
		case rexpr_object_type_STAR:
		case rexpr_object_type_PLUS:
		case rexpr_object_type_ROUND_BRACKETS_OPEN_COPY:
		case rexpr_object_type_ROUND_BRACKETS_OPEN:
			ro->type = rexpr_object_type_SQUARE_BRACKETS_OPEN;
			if(data->brackets_map_end == 0){
				PERR("'rexpr_object_type_SQUARE_BRACKETS_CLOSE' is expected");
				return -1;
			}
			data->brackets_map_end -= 1;
			data->end -= 1;
			while( data->end >= data->start ){
				bytes = get_utf8_letter_size(data->str[data->end]);
				if(bytes == 1){
					switch(rexpr_object_type_to_int(data->str[data->end])){
						case rexpr_object_type_SQUARE_BRACKETS_OPEN:
							if(data->brackets_map_end == 0){
								PERR("'rexpr_object_type_SQUARE_BRACKETS_CLOSE' is expected");
								return -1;
							}
							data->brackets_map_end -= 1;
							break;
						case rexpr_object_type_SQUARE_BRACKETS_CLOSE:
							if( (data->brackets_map_end == 0 && data->brackets_map[0] == rexpr_object_type_SQUARE_BRACKETS_CLOSE)
								|| (data->brackets_map_end > 0 && data->brackets_map[data->brackets_map_end - 1] == rexpr_object_type_SQUARE_BRACKETS_OPEN) ){
								/*Закрываем этот объект*/
								data->end -= 1;
								ro->next = parent->child;
								parent->child = ro;
								data->brackets_map_end -= 1;

								PFUNC_END();
								return 0;
							}
							if(data->brackets_map_end == 0){
								PERR("'rexpr_object_type_SQUARE_BRACKETS_CLOSE' is expected");
								return -1;
							}
							data->brackets_map_end -= 1;
							break;
					}
				}
				if(bytes == 0){
					/*Скорей всего кусок многобайтового символа*/
					data->end -= 1;
					continue;
				}
				if(data->end - 1 >= data->start){
					if(data->str[data->end - 1] == '-'){
						/*Создание области символов*/
						if((data->end - 1 - bytes) >= data->start){
							if(bytes == get_utf8_letter_size(data->str[data->end - 1 - bytes])){
								if(bytes == 1 && data->str[data->end - 2] == rexpr_object_type_to_ch[rexpr_object_type_SQUARE_BRACKETS_CLOSE]){
									if( (data->brackets_map_end == 0 && data->brackets_map[0] == rexpr_object_type_SQUARE_BRACKETS_CLOSE)
										|| (data->brackets_map_end > 0 && data->brackets_map[data->brackets_map_end - 1] == rexpr_object_type_SQUARE_BRACKETS_OPEN) ){
										/*Ситуация когда '[-...'*/
										/*и скобка '[' - закрывает этот объект*/
										goto create_one_char;
									}
								}
								if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, data->str + (data->end - 1 - bytes), data->str + data->end, bytes))
									return -1;
								data->end = data->end - 2 - bytes;
								continue;
							}
						}
					}
					if(bytes == 1 && data->str[data->end - 1] == rexpr_object_type_second_to_ch[rexpr_object_type_second_ESCAPE]){
						/*Создание ecs-символа*/
						esc = rexpr_escape_type_to_int(data->str[data->end]);
						if(esc == rexpr_escape_type_unknown_ch)
							goto create_one_char;
						if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, rexpr_escape_type_to_ch + esc, NULL, 1))
							return -1;
						data->end -= 2;
						continue;
					}
				}
				create_one_char:
				if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN_create_ch(ro, data->str + data->end, NULL, bytes))
					return -1;
				data->end -= 1;
			}
			break;
		default:
			PERR("unexpected parent type");
			return -1;
	}
	return -1;
}

static char parse_rexpr_object_type_ANGLE_BRACKETS_OPEN(rexpr_object * parent, rexpr_object_data * data)
{
	PFUNC_START();
	PRINT("%lld / %lld\n", (long long)data->start, (long long)data->end);
	if(parent == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(data->start > data->end){
		PERR("unexpected start / end: %lld / %lld", (long long)data->start, (long long)data->end);
		return -1;
	}
	rexpr_object * ro;

	ro = malloc(sizeof(rexpr_object));
	if(ro == NULL){
		PCERR("ptr is NULL");
		return -1;
	}
	ro->child = NULL;
	ro->next = NULL;
	rexpr_object * quest;
	uchar_t tmp_end;

	switch(parent->type){
		case rexpr_object_type_STAR:
		case rexpr_object_type_PLUS:
		case rexpr_object_type_ROUND_BRACKETS_OPEN_COPY:
		case rexpr_object_type_ROUND_BRACKETS_OPEN:
			ro->type = rexpr_object_type_null;
			ro->s_type = rexpr_object_type_second_null;
			tmp_end = data->end;
			while(1){
				/*Поиск закрывающей скобки <*/
				tmp_end -= 1;
				if(tmp_end < data->start){
					PERR("unexpected EOL");
					return -1;
				}
				if(data->str[tmp_end] == rexpr_object_type_second_to_ch[rexpr_object_type_second_ANGLE_BRACKETS_CLOSE])
					break;
			}
#if DBG_LVL >= 2
			PRINT("search group by name: ");
			PRINT2(data->str + (tmp_end + 1), data->end - (tmp_end + 1));
			PRINT("\n");
#endif
			quest = (rexpr_object *)get_data_ptree(data->groups, data->str + (tmp_end + 1), data->end - (tmp_end + 1), 0);
			if(quest != NULL){
				if(quest->type == rexpr_object_type_ROUND_BRACKETS_OPEN){
					ro->type = rexpr_object_type_ROUND_BRACKETS_OPEN_COPY;
					ro->s_type = rexpr_object_type_second_ANGLE_BRACKETS_OPEN;
					ro->data.group_id = quest->data.group_id;
					ro->child = quest->child;

					data->end = tmp_end - 1;
					ro->next = parent->child;
					parent->child = ro;

					PFUNC_END();
					return 0;
				}
			}
			if(-1 == insert_data_ptree(data->groups, data->str + (tmp_end + 1), data->end - (tmp_end + 1), (void *)ro)){
				PERR("fault insert to ptree");
				return -1;
			}
			data->end = tmp_end - 1;
			ro->next = parent->child;
			parent->child = ro;

			PFUNC_END();
			return 0;
			break;
		default:
			PERR("unexpected parent type");
			return -1;
	}
	return -1;
}

static char parse_rexpr_object(rexpr_object * parent, rexpr_object_data * data)
{
	/*
		Функция парсит регулярное выражение, создает его представление в структурах
		Возвращает:
			-1	ERR
			0	OK
	*/
	PFUNC_START();
	PRINT("%lld / %lld\n", (long long)data->start, (long long)data->end);
	if(parent == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(data->start > data->end){
		PERR("unexpected start / end: %lld / %lld", (long long)data->start, (long long)data->end);
		return -1;
	}

	PRINT("parse_rexpr_object: parent: %c\n", (char)rexpr_object_type_to_ch[parent->type]);
	switch(parent->type){
		case rexpr_object_type_ROUND_BRACKETS_OPEN_COPY:
		case rexpr_object_type_ROUND_BRACKETS_OPEN:
			while(data->end >= data->start){
				switch(rexpr_object_type_to_int(data->str[data->end])){
					case rexpr_object_type_DOT:
						if(0 != parse_rexpr_object_create_DOT(parent, data))
								return -1;
						break;
					case rexpr_object_type_STAR:
						if(0 != parse_rexpr_object_create_STAR(parent, data))
								return -1;
						break;
					case rexpr_object_type_PLUS:
						if(0 != parse_rexpr_object_create_PLUS(parent, data))
								return -1;
						break;
					case rexpr_object_type_ROUND_BRACKETS_OPEN_COPY:
					case rexpr_object_type_ROUND_BRACKETS_OPEN:
						if(0 != parse_rexpr_object_create_ROUND_BRACKETS_OPEN(parent, data))
								return -1;
						break;
					case rexpr_object_type_ROUND_BRACKETS_CLOSE:
						if(0 != parse_rexpr_object_create_ROUND_BRACKETS_CLOSE(parent, data))
								return -1;
						else
							return 0;
						break;
					case rexpr_object_type_SQUARE_BRACKETS_OPEN:
						if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN(parent, data))
								return -1;
						break;
					case rexpr_object_type_ANGLE_BRACKETS_OPEN:
						if(0 != parse_rexpr_object_type_ANGLE_BRACKETS_OPEN(parent, data))
								return -1;
						break;
					case rexpr_object_type_STRING:
						if(0 != parse_rexpr_object_create_STRING(parent, data))
							return -1;
						break;
					default:
						PERR("unexpected 'rexpr_object_type': %u", (unsigned int)rexpr_object_type_to_int(data->str[data->end]));
						return -1;
				}
			}
			break; //case rexpr_object_type_ROUND_BRACKETS_OPEN
		case rexpr_object_type_PLUS:
		case rexpr_object_type_STAR:
			switch(rexpr_object_type_to_int(data->str[data->end])){
				case rexpr_object_type_DOT:
					if(0 != parse_rexpr_object_create_DOT(parent, data))
							return -1;
					break;
				case rexpr_object_type_ROUND_BRACKETS_OPEN_COPY:
				case rexpr_object_type_ROUND_BRACKETS_OPEN:
					if(0 != parse_rexpr_object_create_ROUND_BRACKETS_OPEN(parent, data))
							return -1;
					break;
				case rexpr_object_type_SQUARE_BRACKETS_OPEN:
					if(0 != parse_rexpr_object_create_SQUARE_BRACKETS_OPEN(parent, data))
							return -1;
					break;
				case rexpr_object_type_ANGLE_BRACKETS_OPEN:
					if(0 != parse_rexpr_object_type_ANGLE_BRACKETS_OPEN(parent, data))
							return -1;
					break;
				case rexpr_object_type_STRING:
					if(0 != parse_rexpr_object_create_STRING(parent, data))
						return -1;
					break;
				default:
					PERR("unexpected 'rexpr_object_type': %u", (unsigned int)rexpr_object_type_to_int(data->str[data->end]));
					return -1;
			}
			break;
		default:
			PERR("unexpected parent type: %u", (unsigned int)parent->type);
			return -1;	
	}
	PFUNC_END();
	return 0;
}

static char init_rexpr_data(rexpr_object_data * data, uchar_t * str, bytes_t start, bytes_t end, ptree_node * groups)
{
	/*
		Функция инициализирует данные для компиляции
		groups - может быть NULL
		Возвращает
			-1	ERR
			0	OK
	*/
	PFUNC_START();
	if(data == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(str == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(0 > end){
		PERR("unexpected end: %lld / %lld", (long long)start, (long long)end);
		return -1;
	}
	if(start > end){
		PERR("unexpected start: %lld / %lld", (long long)start, (long long)end);
		return -1;
	}

	data->str = str;
	data->start = start;
	data->end = end;
	data->group_count = 0;
	if(groups == NULL)
		data->groups = alloc_node_ptree('a');
	else
		data->groups = groups;
	if(data->groups == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	data->brackets_map_end = 0;
	/*Поиск скобок []*/
	data->brackets_map = malloc(sizeof(rexpr_object) * (end - start + 1));
	if(data->brackets_map == NULL){
		PCERR("ptr is NULL");
		return -1;
	}
	PRINT("square brake map: ");
	while(start <= end){
		uchar_t bytes = get_utf8_letter_size(str[start]);
		if(bytes == 1)
			switch(rexpr_object_type_to_int(str[start])){
				case rexpr_object_type_SQUARE_BRACKETS_OPEN:
					data->brackets_map[data->brackets_map_end] = rexpr_object_type_SQUARE_BRACKETS_OPEN;
					data->brackets_map_end += 1;
					PRINT(" ] ");
					break;
				case rexpr_object_type_SQUARE_BRACKETS_CLOSE:
					data->brackets_map[data->brackets_map_end] = rexpr_object_type_SQUARE_BRACKETS_CLOSE;
					data->brackets_map_end += 1;
					PRINT(" [ ");
					break;
			}
		if(bytes != 0)
			start += bytes;
		else
			start += 1;
	}
	PRINT("\n");
	if(data->brackets_map_end == 0){
		free(data->brackets_map);
		data->brackets_map = NULL;
	} else {
		data->brackets_map = realloc(data->brackets_map, sizeof(rexpr_object) * data->brackets_map_end);
		if(data->brackets_map == NULL){
			PCERR("ptr is NULL");
			return -1;
		}
		data->brackets_map_end -= 1; //перевод из размера в позицию
	}
	PFUNC_END();
	return 0;
}

static char free_rexpr_data(rexpr_object_data * data)
{
	/*
		Функция удаляет данные компиляции
		Возвращает
			-1	ERR
			0	OK
	*/
	PFUNC_START();
	if(data == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(data->groups != NULL){
		free_ptree(data->groups);
	}
	if(data->brackets_map != NULL){
		free(data->brackets_map);
	}
	PFUNC_END();
	return 0;
}

long long compile_rexpr(rexpr_object * root, uchar_t * expr, bytes_t expr_len)
{
	/*
		Функция компилирует регулярное выражение
		Возвращает
			позицию последнего символа	OK
			позицию ошибки в строке		ERR1
			-1				ERR2
	*/
	PFUNC_START();
	if(root == NULL){
		PERR("ptr is NULL");
		return (long long)-1;
	}
	bytes_t start = 0;
	bytes_t end = expr_len - 1;
	rexpr_object_data data;

	root->type = rexpr_object_type_ROUND_BRACKETS_OPEN;
	root->s_type = rexpr_object_type_second_ROUND_BRACKETS_CLOSE;
	root->next = NULL;
	root->child = NULL;
	root->data.group_id = 0;

	if(0 != init_rexpr_data(&data, expr, start, end, NULL)){
		return (long long)-1;
	}
	if(0 != parse_rexpr_object(root, &data)){
		end = data.end;
	}
	root->data.group_id = data.group_count;
	free_rexpr_data(&data);
	PFUNC_END();
	return (long long)end;
}

void free_rexpr(rexpr_object * parent)
{
	/*
		Функция освобождает память из под скомпилированного выражения
	*/
	PFUNC_START();
	if(parent == NULL)
		return;

	rexpr_object * child, * c_tmp;
	struct rexpr_object_ch_range * range, * r_tmp;
	
	switch(parent->type){
		case rexpr_object_type_DOT:
			PFUNC_END();
			return;
		case rexpr_object_type_PLUS:
		case rexpr_object_type_STAR:
			child = parent->child;
			while(child != NULL){
				free_rexpr(child);
				c_tmp = child;
				child = child->next;
				free(c_tmp);
			}
			PFUNC_END();
			return;
			break;
		case rexpr_object_type_ROUND_BRACKETS_OPEN_COPY:
			PFUNC_END();
			return;
			break;
		case rexpr_object_type_ROUND_BRACKETS_OPEN:
			child = parent->child;
			while(child != NULL){
				free_rexpr(child);
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

/*
	TODO
	Функция сохраняет скомпилированное выражение в файл
*/

/*
	TODO
	Функция читает скомпилированное выражение из файла
*/