#define DBG_LVL 2
#include "../debug.h"

#include "../encoding/utf_8.h"
#include "types.h"
#include "rexpr.h"

/*
	Функции поиска
	
	Все функции check_* должны возвращать количество байт, удовлетворяющих условиям obj
	Позиция передается в data. Она должна обозначать начало проверки (с символа который находится в этой позиции)
	Функции должны начинаться с PFUNC_START(); и информацией о позиции указателей в data:
		PRINT("%lld / %lld\n", (long long)data->start, (long long)data->end);
	Функции должны завершаться либо сообщением с ошибкой, либо PFUNC_END();
	
	Группы, перед проверкой условий, сохраняют состояние data. При совпадении всех своих правил, обновляют data.
	
static bytes_t check_ (rexpr_object * obj, rexpr_object_result * data)
{
	PFUNC_START();
	PRINT("%lld / %lld\n", (long long)data->start, (long long)data->end);
	if(obj == NULL){
		PERR("ptr is NULL");
		return 0;
	}
	
	PFUNC_END();
	return 0;
}
*/
static char _get_next_str(rexpr_object_result * data)
{
	if(data->get_next_str == NULL)
		return -1;
	uchar_t * old_str = data->str;
	data->get_next_str((char **)&data->str, &data->start, &data->end, &data->line, data->get_str_data);
	if( (void *)data->str == (void *)old_str)
		return -1;
	return 0;
}

static void _save_data(rexpr_object_result * to, rexpr_object_result * from)
{
	memcpy(to, from, sizeof(rexpr_object_result));
}

static void _load_data(rexpr_object_result * to, rexpr_object_result * from)
{
	memcpy(to, from, sizeof(rexpr_object_result));
}

static bytes_t check_rexpr_object_type_SQUARE_BRACKETS_OPEN(rexpr_object * obj, rexpr_object_result * data)
{
	PFUNC_START();
	PRINT("%lld / %lld\n", (long long)data->start, (long long)data->end);
	if(obj == NULL){
		PERR("ptr is NULL");
		return 0;
	}
	rexpr_object_result data_tmp;
	unsigned int bytes;
	unsigned int bytes_part;
	unsigned int bytes_tmp;
	unsigned int pos;
	uchar_t ch[MAX_CH_LEN];
	
	_save_data(&data_tmp, data);
	
	bytes = get_utf8_letter_size(data->str[data->start]);
	if(bytes == 0){
		bytes = 1;
	}
	bytes_tmp = bytes;
	if(data->start + (bytes_t)bytes > data->end){
		/*Данные разбиты на строки*/
		pos = 0;
		while(1){
			bytes_part = (unsigned int)((data->start + (bytes_t)bytes) - data->end);
			bytes = bytes - bytes_part;
			memcpy(ch + pos, data->str + data->start, bytes_part);
			pos += bytes_part;
			if(0 != _get_next_str(data)){
				_load_data(data, &data_tmp);
				PINF("unexpected EOL");
				return 0;
			}
			if(data->start + (bytes_t)bytes > data->end){
				continue;
			} else {
				memcpy(ch + pos, data->str + data->start, bytes);
				data->start += bytes;
				break;
			}
		}
	} else {
		memcpy(ch, data->str + data->start, bytes);
		data->start += bytes;
	}
	bytes = bytes_tmp;
	rexpr_object_ch_range * range = obj->data.ch_range;
	while(range != NULL){
		if(range->bytes != bytes){
			range = range->next;
			continue;
		}
		if(range->r != 0){
			if(0 >= memcmp(range->l, (void *)ch, bytes)){
				if(0 >= memcmp(range->r, (void *)ch, bytes)){
					PFUNC_END();
					return (bytes_t)bytes;
				}
			}
		} else {
			if(0 == memcmp(range->l, (void *)ch, bytes)){
				PFUNC_END();
				return (bytes_t)bytes;
			}
		}
		range = range->next;
	}
	_load_data(data, &data_tmp);
	PFUNC_END();
	return 0;
}

static bytes_t check_rexpr_object_type_STRING(rexpr_object * obj, rexpr_object_result * data)
{
	PFUNC_START();
	PRINT("%lld / %lld\n", (long long)data->start, (long long)data->end);
	if(obj == NULL){
		PERR("ptr is NULL");
		return 0;
	}
	PRINT2(obj->data.str.str, obj->data.str.len);
	PRINT("\n");
	rexpr_object_result data_tmp;
	bytes_t len_part;
	bytes_t pos;
	
	_save_data(&data_tmp, data);
	
	if(data->start + obj->data.str.len > data->end){
		/*Данные разбиты на строки*/
		pos = 0;
		while((obj->data.str.len - pos) != 0){
			len_part = obj->data.str.len - ((data->start + (obj->data.str.len - pos)) - data->end) + 1;
			if(len_part > 0){
				if(0 != memcmp(obj->data.str.str + pos, data->str + data->start, len_part)){
					_load_data(data, &data_tmp);
					PFUNC_END();
					return 0;
				}
				pos += len_part;
			}
			if(0 != _get_next_str(data)){
				_load_data(data, &data_tmp);
				PINF("unexpected EOL");
				return 0;
			}
			if((obj->data.str.len - pos) == 0)
				break;
			if(data->start + (obj->data.str.len - pos) > data->end){
				continue;
			} else {
				if(0 != memcmp(obj->data.str.str + pos, data->str + data->start, (obj->data.str.len - pos))){
					_load_data(data, &data_tmp);
					PFUNC_END();
					return 0;
				}
			}
		}
		data->start += obj->data.str.len - pos;
		return obj->data.str.len;
	} else {
		if(0 == memcmp(obj->data.str.str, data->str + data->start, obj->data.str.len)){
			data->start += obj->data.str.len;
			PFUNC_END();
			return obj->data.str.len;
		}
	}
	_load_data(data, &data_tmp);
	PFUNC_END();
	return 0;
}

static bytes_t check_rexpr_object_type_DOT(rexpr_object * obj, rexpr_object_result * data)
{
	PFUNC_START();
	PRINT("%lld / %lld\n", (long long)data->start, (long long)data->end);
	if(obj == NULL){
		PERR("ptr is NULL");
		return 0;
	}
	rexpr_object_result data_tmp;
	unsigned int bytes;
	unsigned int bytes_tmp;
	
	_save_data(&data_tmp, data);
	
	bytes = get_utf8_letter_size(data->str[data->start]);
	if(bytes == 0){
		bytes = 1;
	}
	if(data->start + (bytes_t)bytes > data->end){
		/*Данные разбиты на строки*/
		bytes_tmp = bytes;
		while(1){
			bytes = bytes - (unsigned int)((data->start + (bytes_t)bytes) - data->end);
			if(0 != _get_next_str(data)){
				_load_data(data, &data_tmp);
				PINF("unexpected EOL");
				return 0;
			}
			if(data->start + (bytes_t)bytes > data->end){
				continue;
			} else {
				//TODO: изменять data->start
				PFUNC_END();
				return bytes_tmp;
			}
		}
	} else {
		data->start += (bytes_t)bytes;
		PFUNC_END();
		return bytes;
	}
	_load_data(data, &data_tmp);
	PFUNC_END();
	return 0;
}
static bytes_t check_rexpr_object_type_ROUND_BRACKETS_OPEN(rexpr_object * obj, rexpr_object_result * data);
static bytes_t check_rexpr_object_type_STAR(rexpr_object * obj, rexpr_object_result * data)
{
	/*
		Используется как STAR и PLUS
	*/
	PFUNC_START();
	PRINT("%lld / %lld\n", (long long)data->start, (long long)data->end);
	if(obj == NULL){
		PERR("ptr is NULL");
		return 0;
	}
	if(obj->child == NULL){
		PERR("ptr is NULL");
		return 0;
	}
	bytes_t len;
	bytes_t ret = 0;
	switch(obj->child->type){
		case rexpr_object_type_DOT:
			while(1){
				len = check_rexpr_object_type_DOT(obj->child, data);
				if(len == 0){
					PFUNC_END();
					return ret;
				}
				ret += len;
			}
			break;
		case rexpr_object_type_STRING:
			while(1){
				len = check_rexpr_object_type_STRING(obj->child, data);
				if(len == 0){
					PFUNC_END();
					return ret;
				}
				ret += len;
			}
			break;
		case rexpr_object_type_SQUARE_BRACKETS_OPEN:
			while(1){
				len = check_rexpr_object_type_SQUARE_BRACKETS_OPEN(obj->child, data);
				if(len == 0){
					PFUNC_END();
					return ret;
				}
				ret += len;
			}
			break;
		case rexpr_object_type_ROUND_BRACKETS_OPEN_COPY:
		case rexpr_object_type_ROUND_BRACKETS_OPEN:
			while(1){
				len = check_rexpr_object_type_ROUND_BRACKETS_OPEN(obj->child, data);
				if(len == -1){
					PFUNC_END();
					return ret;
				}
				ret += len;
			}
			break;
		default:
			PERR("unexpected rexpr_object_type_: %d", (int)obj->child->type);
			return 0;
	}
	PFUNC_END();
	return 0;
}

static bytes_t check_rexpr_object_type_ROUND_BRACKETS_OPEN(rexpr_object * obj, rexpr_object_result * data)
{
	PFUNC_START();
	PRINT("%lld / %lld\n", (long long)data->start, (long long)data->end);
	if(obj == NULL){
		PERR("ptr is NULL");
		return 0;
	}
	rexpr_object_result data_tmp;
	rexpr_object_result_group * rg;
	bytes_t ret;
	bytes_t len;
	rexpr_object * child;
	unsigned short count;
	unsigned int rg_idx;

	_save_data(&data_tmp, data);

	child = obj->child;
	switch(obj->s_type){
		case rexpr_object_type_second_NOT:
			PRINT("type: rexpr_object_type_second_NOT\n");
			/*
				Отрицание. Варианты результата:
					1) Сработали все правила	'-1'
					2) Сработала часть правил	'смещение'
					3) Все правила не сработали	'0'
				"правильные" результаты: 2,3.
			*/
			ret = 0;
			while(child != NULL){
				switch(child->type){
					case rexpr_object_type_DOT:
						len = check_rexpr_object_type_DOT(child, data);
						if(len == 0){
							PFUNC_END();
							return ret;
						}
						ret += len;
					break;
					case rexpr_object_type_STAR:
						len = check_rexpr_object_type_STAR(child, data);
						ret += len;
					break;
					case rexpr_object_type_PLUS:
						len = check_rexpr_object_type_STAR(child, data);
						if(len == 0){
							PFUNC_END();
							return ret;
						}
						ret += len;
					break;
					case rexpr_object_type_STRING:
						len = check_rexpr_object_type_STRING(child, data);
						if(len == 0){
							PFUNC_END();
							return ret;
						}
						ret += len;
					break;
					case rexpr_object_type_SQUARE_BRACKETS_OPEN:
						len = check_rexpr_object_type_SQUARE_BRACKETS_OPEN(child, data);
						if(len == 0){
							PFUNC_END();
							return ret;
						}
						ret += len;
					break;
					case rexpr_object_type_ROUND_BRACKETS_OPEN_COPY:
					case rexpr_object_type_ROUND_BRACKETS_OPEN:
						len = check_rexpr_object_type_ROUND_BRACKETS_OPEN(child, data);
						if(len == -1){
							PFUNC_END();
							return ret;
						}
						ret += len;
					break;
				}
				child = child->next;
			}
			_load_data(data, &data_tmp);
			PFUNC_END();
			return -1;
			break; //case rexpr_object_type_second_NOT
		case rexpr_object_type_second_OR:
			PRINT("type: rexpr_object_type_second_OR\n");
			/*
				Логическое ИЛИ. Варианты результата:
					1) Совпало одно любое правило		'смещение'
					2) Не совпало ни одного				'-1'
				Приоритет совпадения правил идет по порядку (слева на право в паттерне) от большего к меньшему
			*/
			ret = 0;
			while(child != NULL){
				switch(child->type){
					case rexpr_object_type_DOT:
						len = check_rexpr_object_type_DOT(child, data);
						if(len > 0){
							PFUNC_END();
							return len;
						}
					break;
					case rexpr_object_type_STAR:
						len = check_rexpr_object_type_STAR(child, data);
						PFUNC_END();
						return len;
					break;
					case rexpr_object_type_PLUS:
						len = check_rexpr_object_type_STAR(child, data);
						if(len > 0){
							PFUNC_END();
							return len;
						}
					break;
					case rexpr_object_type_STRING:
						len = check_rexpr_object_type_STRING(child, data);
						if(len > 0){
							PFUNC_END();
							return len;
						}
					break;
					case rexpr_object_type_SQUARE_BRACKETS_OPEN:
						len = check_rexpr_object_type_SQUARE_BRACKETS_OPEN(child, data);
						if(len > 0){
							PFUNC_END();
							return len;
						}
					break;
					case rexpr_object_type_ROUND_BRACKETS_OPEN_COPY:
					case rexpr_object_type_ROUND_BRACKETS_OPEN:
						len = check_rexpr_object_type_ROUND_BRACKETS_OPEN(child, data);
						if(len >= 0){
							PFUNC_END();
							return len;
						}
					break;
				}
				child = child->next;
			}
			_load_data(data, &data_tmp);
			PFUNC_END();
			return -1;
			break; //case rexpr_object_type_second_OR
		case rexpr_object_type_second_BRACE_OPEN:
			PRINT("type: rexpr_object_type_second_BRACE_OPEN\n");
			/*
				Область повторений. Варианты результата:
					1) Количество повторений группы попало в область {l,r}		'смещение'
					2) Не попало в область										'-1'
					3) Правила вообще не сработали								'-1'
			*/
			ret = 0;
			count = 0;
			while(1){
				while(child != NULL){
					switch(child->type){
						case rexpr_object_type_DOT:
							len = check_rexpr_object_type_DOT(child, data);
							if(len == 0){
								if(count < obj->data.len_range.l || obj->data.len_range.r < count){
									ret = -1;
									_load_data(data, &data_tmp);
								}
								PFUNC_END();
								return ret;
							}
							ret += len;
						break;
						case rexpr_object_type_STAR:
							len = check_rexpr_object_type_STAR(child, data);
							if(obj->data.len_range.r < count){
									_load_data(data, &data_tmp);
									PFUNC_END();
									return -1;
							}
							ret += len;
						break;
						case rexpr_object_type_PLUS:
							len = check_rexpr_object_type_STAR(child, data);
							if(len == 0){
								if(count < obj->data.len_range.l || obj->data.len_range.r < count){
									ret = -1;
									_load_data(data, &data_tmp);
								}
								PFUNC_END();
								return ret;
							}
							ret += len;
						break;
						case rexpr_object_type_STRING:
							len = check_rexpr_object_type_STRING(child, data);
							if(len == 0){
								if(count < obj->data.len_range.l || obj->data.len_range.r < count){
									ret = -1;
									_load_data(data, &data_tmp);
								}
								PFUNC_END();
								return ret;
							}
							ret += len;
						break;
						case rexpr_object_type_SQUARE_BRACKETS_OPEN:
							len = check_rexpr_object_type_SQUARE_BRACKETS_OPEN(child, data);
							if(len == 0){
								if(count < obj->data.len_range.l || obj->data.len_range.r < count){
									ret = -1;
									_load_data(data, &data_tmp);
								}
								PFUNC_END();
								return ret;
							}
							ret += len;
						break;
						case rexpr_object_type_ROUND_BRACKETS_OPEN_COPY:
						case rexpr_object_type_ROUND_BRACKETS_OPEN:
							len = check_rexpr_object_type_ROUND_BRACKETS_OPEN(child, data);
							if(len == -1){
								if(count < obj->data.len_range.l || obj->data.len_range.r < count){
									ret = -1;
									_load_data(data, &data_tmp);
								}
								PFUNC_END();
								return ret;
							}
							ret += len;
						break;
					}
					child = child->next;
				}
				child = obj->child;
				count += 1;
				if(obj->data.len_range.r < count){
						_load_data(data, &data_tmp);
						PFUNC_END();
						return -1;
				}
			}
			break; //case rexpr_object_type_second_BRACE_OPEN
		case rexpr_object_type_second_ROUND_BRACKETS_CLOSE:
		case rexpr_object_type_second_ANGLE_BRACKETS_OPEN:
			PRINT("type: rexpr_object_type_second_ROUND_BRACKETS_CLOSE\n");
			/*
				Простая группа.  Варианты результата:
					1) Все группы совпали		'смещение'
					2) Любая не совпала			'-1'
			*/
			ret = 0;
			while(child != NULL){
				switch(child->type){
					case rexpr_object_type_DOT:
						len = check_rexpr_object_type_DOT(child, data);
						if(len == 0){
							_load_data(data, &data_tmp);
							PFUNC_END();
							return -1;
						}
						ret += len;
					break;
					case rexpr_object_type_STAR:
						len = check_rexpr_object_type_STAR(child, data);
						ret += len;
					break;
					case rexpr_object_type_PLUS:
						len = check_rexpr_object_type_STAR(child, data);
						if(len == 0){
							_load_data(data, &data_tmp);
							PFUNC_END();
							return -1;
						}
						ret += len;
					break;
					case rexpr_object_type_STRING:
						len = check_rexpr_object_type_STRING(child, data);
						if(len == 0){
							_load_data(data, &data_tmp);
							PFUNC_END();
							return -1;
						}
						ret += len;
					break;
					case rexpr_object_type_SQUARE_BRACKETS_OPEN:
						len = check_rexpr_object_type_SQUARE_BRACKETS_OPEN(child, data);
						if(len == 0){
							_load_data(data, &data_tmp);
							PFUNC_END();
							return -1;
						}
						ret += len;
					break;
					case rexpr_object_type_ROUND_BRACKETS_OPEN_COPY:
					case rexpr_object_type_ROUND_BRACKETS_OPEN:
						len = check_rexpr_object_type_ROUND_BRACKETS_OPEN(child, data);
						if(len == -1){
							_load_data(data, &data_tmp);
							PFUNC_END();
							return -1;
						}
						ret += len;
					break;
				}
				child = child->next;
			}
			if(obj->s_type == rexpr_object_type_second_ANGLE_BRACKETS_OPEN && data->group_result != NULL){
				rg_idx = data->group_result_size - obj->data.group_id + 1;
				if(data->group_result_size < rg_idx || rg_idx <= 0){
					_load_data(data, &data_tmp);
					PERR("unexpected index: %u / [1,%u]", rg_idx, data->group_result_size);
					return -1;
				}
				rg = malloc(sizeof(rexpr_object_result_group));
				if(rg == NULL){
					PCERR("ptr is NULL");
					_load_data(data, &data_tmp);
					return -1;
				}
				rg->line_start = data_tmp.line;
				rg->start = data_tmp.start;
				rg->line_end = data->line;
				rg->end = data->start;
				if(data->group_result[rg_idx] != NULL){
					rg->next = data->group_result[rg_idx];
				}
				data->group_result[rg_idx] = rg;
			}
			return ret;
			break; // case "default"
		default:
			_load_data(data, &data_tmp);
			PERR("unexpected rexpr_object_type_second_: %d", (int)obj->s_type);
			return -1;
	}
	_load_data(data, &data_tmp);
	PFUNC_END();
	return -1;
}

long long check_str_rexpr_object(rexpr_object * pattern, char * str, bytes_t str_len, rexpr_object_result * result)
{
	/*
		Функция сравнивает строку str с паттерном pattern
		Если result не NULL, то туда запишуться дополнительные результаты сравнения (например данные по группам)
		После, структуру result необходимо очистить соответствующей функцией
		Возвращает:
				позицию после последнего совпавшего символа последней совпавшей строки		OK
				-1																		ERR
	*/
	PFUNC_START();
	rexpr_object_result data;
	if(result == NULL){
		result = &data;
		init_rexpr_object_result(result, NULL, NULL);
	} else {
		clear_rexpr_object_result(result);
		if(pattern->data.group_id > 0){
			result->group_result = malloc(sizeof(rexpr_object_result_group *) * (pattern->data.group_id + 1));
			if(result->group_result == NULL){
				PERR("ptr is NULL");
				return -1;
			}
			result->group_result_size = pattern->data.group_id;
		}
	}
	result->str = (uchar_t *)str;
	result->end = str_len - 1;
	if(-1 == check_rexpr_object_type_ROUND_BRACKETS_OPEN(pattern, result)){
		PFUNC_END();
		return -1;
	}
	PFUNC_END();
	return result->start;
}

char init_rexpr_object_result(rexpr_object_result * result, 
				void (* get_next_str)(char ** str, ssize_t * start, ssize_t * end, unsigned int * line, void * get_str_data),
				void * get_str_data)
{
	/*
		Инициализирует структуру result.
		get_next_str и data могут быть NULL
		Возвращает:
				-1		ERR
				0		OK
	*/
	PFUNC_START();
	if(result == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	result->str = NULL;
	result->start = 0;
	result->end = 0;
	result->group_result = NULL;
	result->group_result_size = 0;
	result->line = 0;
	result->get_next_str = get_next_str;
	result->get_str_data = get_str_data;
	PFUNC_END();
	return 0;
}

void clear_rexpr_object_result(rexpr_object_result * result)
{
	/*
		Очищает структуру result
	*/
	PFUNC_START();
	if(result == NULL){
		return;
	}
	result->str = NULL;
	result->start = 0;
	result->end = 0;
	if(result->group_result_size > 0 && result->group_result != NULL)
		free(result->group_result);
	result->group_result = NULL;
	result->group_result_size = 0;
	result->line = 0;
	PFUNC_END();
}