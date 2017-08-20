#define DBG_LVL 1
#include "../debug.h"

#include "prefix_tree.h"

static ptree_node * get_way(ptree_node * node, unsigned char * keys, unsigned int * idx)
{
	/*
		Определяет путь в дереве
		Возвращает указатель на последний совпавший узел
		В idx будет записана длина пути
	*/

	unsigned int keys_len = *idx;
	ptree_node * head = NULL;

	for(*idx = 0; *idx < keys_len;){
		if(node == NULL)
			return head;
		if(node->key == keys[*idx]){
			head = node;
			node = node->child;
			*idx += 1;
		} else {
			node = node->next;
		}
	}

	return head;
}

static int insert_to_data(ptree_node * node, void * data)
{
	/*
		Добавляет данные в node
		Возвращает
			индекс в массиве data		OK
			-1				ERR
	*/
#ifndef PREFIX_TREE_USE_MULTIDATA
	node->data = data;
	return 0;
#else
	unsigned int idx;

	if(node->data_size == 0){
		/*Массив еще не был создан*/
		node->data = malloc(sizeof(void *) * PREFIX_TREE_INIT_DATA_SIZE);
		if(node->data == NULL){
			PCERR("ptr is NULL");
			return -1;
		}
		node->data_size = PREFIX_TREE_INIT_DATA_SIZE;
		bzero(node->data, PREFIX_TREE_INIT_DATA_SIZE);

		node->data[0] = data;
		return 0;
	}

	for(idx = 0; idx < node->data_size; idx++){
		if(node->data[idx] == NULL){
			node->data[idx] = data;
			return idx;
		}
	}

	node->data = realloc(node->data, sizeof(void *) * (node->data_size + PREFIX_TREE_INIT_DATA_SIZE));
	if(node->data == NULL){
		PCERR("ptr is NULL");
		node->data_size = 0;
		return -1;
	}
	node->data_size = node->data_size + PREFIX_TREE_INIT_DATA_SIZE;
	node->data[idx] = data;
	return idx;
#endif
}

int erase_data_from_node_ptree(ptree_node * node
#ifdef PREFIX_TREE_USE_MULTIDATA
			, unsigned int idx
#endif
			)
{
	/*
		Удаляет данные в node
		Возвращает
			0		OK
			-1		ERR
	*/
#ifndef PREFIX_TREE_USE_MULTIDATA
	node->data = NULL;
#else
	if(node->data_size <= idx){
		return -1;
	}

	node->data[idx] = NULL;

	/*Освобождаем память*/
	for(idx = node->data_size - 1; idx != 0; idx--){
		if(node->data[idx] != NULL)
			break;
	}
	if(node->data[idx] == NULL){
		/*Весь массив пуст*/
		free(node->data);
		node->data = NULL;
		node->data_size = 0;
		return 0;
	}

	idx += 1;//перевод индекса в размер
	if(idx < node->data_size)
		return 0;
	node->data = realloc(node->data, sizeof(void *) * (idx * PREFIX_TREE_INIT_DATA_SIZE));
	if(node->data == NULL){
		PCERR("ptr is NULL");
		node->data_size = 0;
		return -1;
	}
	node->data_size = idx * PREFIX_TREE_INIT_DATA_SIZE;

	return 0;
#endif
}

ptree_node * alloc_node_ptree(unsigned char key)
{
	/*
		Создает узел
	*/
	ptree_node * node = NULL;
	
	node = malloc(sizeof(ptree_node));
	if(node == NULL){
		PCERR("ptr is NULL");
		return NULL;
	}
	node->key = key;
#ifdef PREFIX_TREE_USE_MULTIDATA
	node->data = NULL;
	node->data_size = 0;
#endif
	node->child = NULL;
	node->next = NULL;
	return node;
}

ptree_node * get_node_ptree(ptree_node * head, unsigned char * keys, unsigned int keys_len)
{
	/*
		Возвращает узел по пути keys
	*/
	PFUNC_START();
	if(head == NULL){
		PERR("ptr is NULL");
		return NULL;
	}
	if(keys == NULL){
		PERR("ptr is NULL");
		return NULL;
	}
	unsigned int new_len = keys_len;

	head = get_way(head, keys, &new_len);
	PFUNC_END();
	if(head != NULL && new_len == keys_len){
		return head;
	} else
		return NULL;
}

void * get_data_ptree(ptree_node * head, unsigned char * keys
			, unsigned int keys_len
#ifdef PREFIX_TREE_USE_MULTIDATA
			, unsigned int idx
#endif
			)
{
	/*
		Возвращает данные по пути keys и индексу
	*/
	PFUNC_START();
	if(head == NULL){
		PERR("ptr is NULL");
		return NULL;
	}
	if(keys == NULL){
		PERR("ptr is NULL");
		return NULL;
	}
	unsigned int new_len = keys_len;

	head = get_way(head, keys, &new_len);
	PFUNC_END();
	if(head != NULL && new_len == keys_len){
#ifndef PREFIX_TREE_USE_MULTIDATA
		return head->data;
#else
		if(head->data_size <= idx)
			return NULL;
		return head->data[idx];
#endif
	} else
		return NULL;
}

void * replace_data_ptree(ptree_node * head, unsigned char * keys, unsigned int keys_len
#ifdef PREFIX_TREE_USE_MULTIDATA
				, unsigned int idx
#endif
				, void * data)
{
	/*
		Заменяет данные по пути keys и индексу
		Возвращает:
			указатель на старые данные	OK
			data				ERR
	*/
	PFUNC_START();
	if(head == NULL){
		PERR("ptr is NULL");
		return data;
	}
	if(keys == NULL){
		PERR("ptr is NULL");
		return data;
	}

	void * old_data = NULL;
	ptree_node * node;
	unsigned int new_len = keys_len;

	node = get_way(head, keys, &new_len);
	if(node != NULL && new_len == keys_len){
#ifndef PREFIX_TREE_USE_MULTIDATA
		old_data = node->data;
		node->data = data;
#else
		if(node->data_size <= idx)
			return data;
		old_data = node->data[idx];
		node->data[idx] = data;
#endif
		PFUNC_END();
		return old_data;
	}

	return data;
}

int insert_data_ptree(ptree_node * head, unsigned char * keys, unsigned int keys_len, void * data)
{
	/*
		Добавляет данные по пути keys
		Возвращает:
			индекс в data		OK
			-1			ERR
	*/
	PFUNC_START();
	if(head == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(keys == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(keys_len == 0){
		PERR("unexpected keys_len: %u", keys_len);
		return -1;
	}

	ptree_node * node;
	int idx = -1;
	unsigned int new_len = keys_len;

	node = get_way(head, keys, &new_len);
	if(new_len == keys_len){
		idx = insert_to_data(node, data);

		PFUNC_END();
		return idx;
	}

	/*Создание пути*/
	if(new_len > 0)
		head = node;
	node = alloc_node_ptree(keys[new_len]);
	if(node == NULL)
		return -1;
	node->next = head->next;
	head->next = node;
	new_len += 1;
	for(;new_len < keys_len; new_len++){
		node->child = alloc_node_ptree(keys[new_len]);
		if(node->child == NULL)
			return -1;
		node = node->child;
	}
	idx = insert_to_data(node, data);

	PFUNC_END();
	return idx;
}

void free_node_ptree(ptree_node * node)
{
	/*
		Освобождает память из-под узла
	*/
	PFUNC_START();
	if(node == NULL){
		return;
	}
	
#ifdef PREFIX_TREE_USE_MULTIDATA
	if(node->data_size > 0)
		free(node->data);
#endif
	free(node);
	PFUNC_END();
}

void free_ptree(ptree_node * head)
{
	/*
		Освобождает память всего дерева
	*/
	PFUNC_START();
	if(head == NULL){
		PERR("ptr is NULL");
		return;
	}

	ptree_node * goto_last(ptree_node * node)
	{
		ptree_node * last = NULL;
		while(node != NULL){
			last = node;
			node = node->next;
		}
		return last;
	}

	ptree_node * node = head;
	ptree_node * last = goto_last(node);
	while(last != NULL){
		/*Выстраиваем узлы в список*/
		if(node->child != NULL){
			last->next = node->child;
			node->child = NULL;
		}
		last = goto_last(node->next);
		node = node->next;
	}

	while(head != NULL){
		/*Удаляем узлы*/
		node = head;
		head = head->next;
		free_node_ptree(node);
	}

	PFUNC_END();
}