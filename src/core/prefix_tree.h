#ifndef _PREFIX_TREE_H_
#define _PREFIX_TREE_H_

#include <stdlib.h>
#include <string.h>

/*
	Префиксное дерево.
	Использует однобайтовые символы.
*/
#define PREFIX_TREE_USE_MULTIDATA		//возможность добавлять несколько данных по одному ключу

#ifdef PREFIX_TREE_USE_MULTIDATA
#define PREFIX_TREE_INIT_DATA_SIZE 1
#endif

typedef struct ptree_node ptree_node;
struct ptree_node {
	ptree_node * next;
	ptree_node * child;

	unsigned char key;
#ifndef PREFIX_TREE_USE_MULTIDATA
	void * data;
#else
/*
	data - массив указателей на данные
	Для доступа к данным необходима пара <keys, idx>, где
		keys		символьный ключ
		idx		индекс данных в массиве
*/
	void ** data;
	unsigned int data_size;
#endif

};


/*
	Удаляет данные в node
	Возвращает
		0		OK
		-1		ERR
*/
int erase_data_from_node_ptree(ptree_node * node
#ifdef PREFIX_TREE_USE_MULTIDATA
			, unsigned int idx
#endif
			);

/*
	Создает узел
*/
ptree_node * alloc_node_ptree(unsigned char key);

/*
	Возвращает узел по пути keys
*/
ptree_node * get_node_ptree(ptree_node * head, unsigned char * keys, unsigned int keys_len);

/*
	Возвращает данные по пути keys и индексу
*/
void * get_data_ptree(ptree_node * head, unsigned char * keys
			, unsigned int keys_len
#ifdef PREFIX_TREE_USE_MULTIDATA
			, unsigned int idx
#endif
			);

/*
	Заменяет данные по пути keys и индексу
	Возвращает:
		указатель на старые данные	OK
		data				ERR
*/
void * replace_data_ptree(ptree_node * head, unsigned char * keys, unsigned int keys_len
#ifdef PREFIX_TREE_USE_MULTIDATA
				, unsigned int idx
#endif
				, void * data);

/*
	Добавляет данные по пути keys
	Возвращает:
		индекс в data		OK
		-1			ERR
*/
int insert_data_ptree(ptree_node * head, unsigned char * keys, unsigned int keys_len, void * data);

/*
	Освобождает память из-под узла
*/
void free_node_ptree(ptree_node * node);

/*
	Освобождает память всего дерева
*/
void free_ptree(ptree_node * head);

#endif