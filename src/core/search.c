#define DBG_LVL 1
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
static char _get_str_by_idx(unsigned int idx, rexpr_object_result * data, unsigned char without_pos)
{
	PFUNC_START();
	if(data->get_str_by_idx == NULL)
		return -1;
	PRINT("old: %u'", data->line);
	PRINT2(data->str, data->end + 1);
	PRINT("'\n");
	if(without_pos){
		bytes_t start = data->start;
		bytes_t end = data->end;
		data->get_str_by_idx((char **)&data->str, &start, &end, idx, data->get_str_data);
	} else {
		data->get_str_by_idx((char **)&data->str, &data->start, &data->end, idx, data->get_str_data);
	}
	if( (void *)data->str == (void *)NULL)
		return -1;
	data->line = idx;
	PRINT("new %u: '", data->line);
	PRINT2(data->str, data->end + 1);
	PRINT("'\n");
	PFUNC_END();
	return 0;
}

static char _get_next_str(rexpr_object_result * data)
{
	return _get_str_by_idx(data->line + 1, data, 0);
}

static char _get_next_str_loop(rexpr_object_result * data)
{
	/*
		Переходит на новую строку пока start > end
		Синхронизирует start со строкой
	*/
	PFUNC_START();
	if(data->start <= data->end){
		PFUNC_END();
		return 0;
	}
	bytes_t tmp;

	while(1){
		tmp = (data->start - data->end) - 1;
		if(tmp < 0)
			break;
		if(0 != _get_next_str(data))
			return -1;
		data->start += tmp;
	}
	PFUNC_END();
	return 0;
}

static void _save_data(rexpr_object_result * to, rexpr_object_result * from)
{
	memcpy(to, from, sizeof(rexpr_object_result));
}

static void _load_data(rexpr_object_result * to, rexpr_object_result * from)
{
	if(to->line == from->line){
		memcpy(to, from, sizeof(rexpr_object_result));
	} else {
		memcpy(to, from, sizeof(rexpr_object_result));
		_get_str_by_idx(to->line, to, 1);
	}
}

/*
	Правила для функций сравнения байт и символов:
		check_rexpr_object_type_SQUARE_BRACKETS_OPEN
		check_rexpr_object_type_STRING
		check_rexpr_object_type_DOT

	1) Перед выполнением необходимо сохранять данные о тек.строке
	2) После сохранения данных, необходимо сдвинуть позицию start на байт вперед
	3) Во время проверки изменяются данные о тек.строке
	4) В случае успешного сравнения:
		4.1) Функция возвращает количество совпавших байт
		4.2) Данные о тек.строке должны содержать:
			4.2.1) Индекс строки в которой совпал последний байт
			4.2.2) Позиция последнего совпавшего байта
	5) В случае ошибки:
		5.1) Восстанавливаются данные о тек.строке
		5.2) Возвращается 0
*/

static bytes_t check_rexpr_object_type_SQUARE_BRACKETS_OPEN(rexpr_object * obj, rexpr_object_result * data)
{
	PFUNC_START();
	PRINT("%lld / %lld\n", (long long)data->start, (long long)data->end);
	if(obj == NULL){
		PERR("ptr is NULL");
		return 0;
	}
	rexpr_object_result data_tmp;
	rexpr_object_ch_range * range;
	unsigned int bytes;
	bytes_t offset;
	uchar_t ch[MAX_CH_LEN];

	_save_data(&data_tmp, data);

	data->start += 1;

	if(0 != _get_next_str_loop(data)){
		_load_data(data, &data_tmp);
		PINF("unexpected EOL");
		return 0;
	}

	bytes = get_utf8_letter_size(data->str[data->start]);
	if(bytes == 0){
		/*_load_data(data, &data_tmp);
		PINF("unexpected utf-8 simbol");
		return 0;*/
		bytes = 1;
	}
	range = obj->data.ch_range;
	while(range != NULL){
		if(range->bytes == bytes){
			break;
		}
		range = range->next;
	}
	if(range == NULL){
		_load_data(data, &data_tmp);
		PFUNC_END();
		return 0;
	}

	offset = 0;
	while(1){
		if(data->start == data->end){
			memcpy(ch + offset, data->str + data->start, 1);
			bytes -= 1;
			if(bytes <= 0){
				break;
			} else {
				if(0 != _get_next_str(data)){
					_load_data(data, &data_tmp);
					PINF("unexpected EOL");
					return 0;
				}
				offset += 1;
				continue;
			}
		}
		if((data->start + bytes - 1) > data->end){
			bytes_t tmp = data->end - data->start + 1;
			memcpy(ch + offset, data->str + data->start, tmp);
			bytes -= tmp;
			offset += tmp;
			if(0 != _get_next_str(data)){
				_load_data(data, &data_tmp);
				PINF("unexpected EOL");
				return 0;
			}
			continue;
		}
		memcpy(ch + offset, data->str + data->start, bytes);
		data->start += bytes - 1;
		break;
	}

	PRINT("compare: '");
	PRINT2(ch, range->bytes);
	PRINT("'\n");
	while(range != NULL){
		if(range->r[0] == 0){
			PRINT("range: '");
			PRINT2(range->l, range->bytes);
			PRINT("'\n");
			if(0 == memcmp(range->l, ch, range->bytes)){
				PFUNC_END();
				return range->bytes;
			}
		} else {
			PRINT("range: '");
			PRINT2(range->l, range->bytes);
			PRINT("' ... '");
			PRINT2(range->r, range->bytes);
			PRINT("'\n");
			if(0 >= memcmp(range->l, ch, range->bytes)){
					if(0 <= memcmp(range->r, ch, range->bytes)){
							PFUNC_END();
							return range->bytes;
					}
			}
		}
		range = range->next;
	}
	PRINT("restore\n");
	_load_data(data, &data_tmp);
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
	PRINT("string: '");
	PRINT2(obj->data.str.str, obj->data.str.len);
	PRINT("'\n");
	if(obj->data.str.len <= 0)
		return 0;

	rexpr_object_result data_tmp;
	bytes_t len;
	bytes_t offset;

	_save_data(&data_tmp, data);

	data->start += 1;

	if(0 != _get_next_str_loop(data)){
		_load_data(data, &data_tmp);
		PINF("unexpected EOL");
		return 0;
	}

	len = obj->data.str.len;
	offset = 0;
	while(1){
		if(data->start == data->end){
			PRINT("data->start == data->end: '");
			PRINT2(obj->data.str.str + offset, 1);
			PRINT("' '");
			PRINT2(data->str + data->start, 1);
			PRINT("'\n");
			if(0 != memcmp(obj->data.str.str + offset, data->str + data->start, 1)){
				_load_data(data, &data_tmp);
				PFUNC_END();
				return 0;
			}
			len -= 1;
			if(len <= 0){
				PFUNC_END();
				return obj->data.str.len;
			} else {
				if(0 != _get_next_str(data)){
					_load_data(data, &data_tmp);
					PINF("unexpected EOL");
					return 0;
				}
				offset += 1;
				continue;
			}
		}
		if((data->start + len - 1) > data->end){
			bytes_t tmp = data->end - data->start + 1;
			PRINT("(data->start + len) > data->end: '");
			PRINT2(obj->data.str.str + offset, tmp);
			PRINT("' '");
			PRINT2(data->str + data->start, tmp);
			PRINT("'\n");
			if(0 != memcmp(obj->data.str.str + offset, data->str + data->start, tmp)){
				_load_data(data, &data_tmp);
				PFUNC_END();
				return 0;
			}
			len -= tmp;
			offset += tmp;
			if(0 != _get_next_str(data)){
				_load_data(data, &data_tmp);
				PINF("unexpected EOL");
				return 0;
			}
			continue;
		}
		PRINT("(data->start + len) <= data->end: '");
		PRINT2(obj->data.str.str + offset, len);
		PRINT("' '");
		PRINT2(data->str + data->start, len);
		PRINT("'\n");
		if(0 != memcmp(obj->data.str.str + offset, data->str + data->start, len)){
			_load_data(data, &data_tmp);
			PFUNC_END();
			return 0;
		}
		data->start += len - 1;
		PFUNC_END();
		return obj->data.str.len;
	}
	_load_data(data, &data_tmp);
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

	data->start += 1;

	if(0 != _get_next_str_loop(data)){
		_load_data(data, &data_tmp);
		PINF("unexpected EOL");
		return 0;
	}

	bytes = get_utf8_letter_size(data->str[data->start]);
	if(bytes == 0){
		bytes = 1;
	}
	bytes_tmp = bytes;
	while(1){
		if(data->start == data->end){
			bytes -= 1;
			if(bytes <= 0){
				PFUNC_END();
				return bytes_tmp;
			} else {
				if(0 != _get_next_str(data)){
					_load_data(data, &data_tmp);
					PINF("unexpected EOL");
					return 0;
				}
				continue;
			}
		}
		if((data->start + bytes - 1) > data->end){
			bytes -= data->end - data->start + 1;
			if(0 != _get_next_str(data)){
				_load_data(data, &data_tmp);
				PINF("unexpected EOL");
				return 0;
			}
			continue;
		}
		data->start += bytes - 1;
		PFUNC_END();
		return bytes_tmp;
	}
	_load_data(data, &data_tmp);
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
				if(data_tmp.line == data->line && data_tmp.start == data->start)
					return ret;
				rg_idx = data->group_result_size - obj->data.group_id;
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
				data_tmp.start += 1;
				if(0 != _get_next_str_loop(&data_tmp)){
					PINF("unexpected EOL");
					return 0;
				}
				rg->line_start = data_tmp.line;
				rg->line_end = data->line;
				rg->start = data_tmp.start;
				rg->end = data->start;
				rg->next = NULL;
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

static void free_group_result(rexpr_object_result * result)
{
	if(result->group_result != NULL){
		unsigned int g_idx;
		for(g_idx = 0; g_idx < result->group_result_size; g_idx++){
			rexpr_object_result_group * g_res = result->group_result[g_idx];
			while(g_res != NULL){
				rexpr_object_result_group * g_tmp = g_res;
				g_res = g_res->next;
				free(g_tmp);
			}
		}
		free(result->group_result);
	}
	result->group_result = NULL;
	result->group_result_size = 0;
}

long long check_str_rexpr_object3(rexpr_object * pattern, char * str, bytes_t str_len)
{
	rexpr_object_result result;
	long long res;

	init_rexpr_object_result(&result, str, str_len, NULL, NULL);
	res = check_str_rexpr_object(pattern, &result);
	clear_rexpr_object_result(&result);

	return res;
}

long long check_str_rexpr_object(rexpr_object * pattern, rexpr_object_result * result)
{
	/*
		Функция сравнивает строку str с паттерном pattern
		Возвращает:
				позицию после последнего совпавшего символа последней совпавшей строки		OK
				-1																			ERR
	*/
	PFUNC_START();
	if(result == NULL){
		PERR("ptr is NULL");
		return -1;
	}

	free_group_result(result);

	if(pattern->data.group_id > 0){
		result->group_result = malloc(sizeof(rexpr_object_result_group *) * (pattern->data.group_id + 1));
		if(result->group_result == NULL){
			PERR("ptr is NULL");
			return -1;
		}
		bzero(result->group_result, sizeof(rexpr_object_result_group *) * (pattern->data.group_id + 1));
		result->group_result_size = pattern->data.group_id + 1;
	}
	if(-1 == check_rexpr_object_type_ROUND_BRACKETS_OPEN(pattern, result)){
		PFUNC_END();
		return -1;
	}
	PFUNC_END();
	return result->start;
}

char init_rexpr_object_result(rexpr_object_result * result, char * str, bytes_t str_len,
				void (* get_str_by_idx)(char ** str, ssize_t * start, ssize_t * end, unsigned int line_idx, void * get_str_data),
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
	result->str = (uchar_t *)str;
	result->start = -1;
	result->end = str_len - 1;
	result->group_result = NULL;
	result->group_result_size = 0;
	result->line = 0;
	result->get_str_by_idx = get_str_by_idx;
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
	free_group_result(result);
	result->line = 0;
	PFUNC_END();
}
