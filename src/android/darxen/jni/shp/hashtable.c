
#include "hashtable.h"

#include <malloc.h>
#include <stdlib.h>

#define FALSE 0
#define TRUE !FALSE

#define LOAD_MAX	0.8

typedef struct _Node	Node;

struct _Node
{
	pointer key;
	pointer val;
	Node* next;
};

struct _HashTable
{
	HashFunc hashFunc;
	EqualsFunc equalsFunc;
	Node** table;
	int capacity;
	int size;
};

typedef struct 
{
	HashTable* table;
	int index;
	Node* pNode;
} HashTableIterPriv;

static void check_rehash(HashTable* table);
static void clear_table(HashTable* table, int freeNodes);

HashTable* hash_table_new(HashFunc hashFunc, EqualsFunc equalsFunc)
{
	HashTable* table = (HashTable*)malloc(sizeof(HashTable));
	table->hashFunc = hashFunc;
	table->equalsFunc = equalsFunc;
	table->size = 0;
	table->capacity = 2;
	table->table = (Node**)calloc(table->capacity, sizeof(Node*));
	clear_table(table, FALSE);
}

void hash_table_free(HashTable* table)
{
	clear_table(table, TRUE);
	free(table->table);
	free(table);
}

void hash_table_insert(HashTable* table, pointer key, pointer val)
{
	int i = table->hashFunc(key) % table->capacity;
	//printf("Insert index: %i\n", i);
	Node* p = (Node*)malloc(sizeof(Node));
	p->key = key;
	p->val = val;
	p->next = table->table[i];
	/*if (p->next)
		printf("Insert collision!\n");*/
	table->table[i] = p;
	table->size++;
	check_rehash(table);
}

pointer hash_table_lookup(HashTable* table, pointer key)
{
	int i = table->hashFunc(key) % table->capacity;
	Node* pNode = table->table[i];
	while (pNode)
	{
		if (table->equalsFunc(key, pNode->key))
			return pNode->val;
		pNode = pNode->next;
	}
	return NULL;
}

pointer hash_table_remove(HashTable* table, pointer key)
{
	int i = table->hashFunc(key) % table->capacity;
	Node* pNode = table->table[i];
	Node* pPrevNode = NULL;
	while (pNode)
	{
		if (table->equalsFunc(key, pNode->key))
		{
			pointer val = pNode->val;
			if (pPrevNode)
				pPrevNode->next = pNode->next;
			else
				table->table[i] = pNode->next;
			free(pNode);
			table->size++;
			return val;
		}
		pPrevNode = pNode;
		pNode = pNode->next;
	}
	return NULL;
}

static void check_rehash(HashTable* table)
{
	float load = (float)table->size / table->capacity;
	if (load < LOAD_MAX)
		return;
	int oldCapacity = table->capacity;
	Node** oldTable = table->table;
	table->capacity *= 2;
	table->table = (Node**)calloc(table->capacity, sizeof(Node*));
	clear_table(table, FALSE);
	int i;
	for (i = 0; i < oldCapacity; i++)
	{
		Node* pNode = oldTable[i];
		while (pNode)
		{
			hash_table_insert(table, pNode->key, pNode->val);
			Node* pNodeC = pNode;
			pNode = pNode->next;
			free(pNodeC);
		}
	}
	free(oldTable);
}

static void clear_table(HashTable* table, int freeNodes)
{
	int i;
	for (i = 0; i < table->capacity; i++)
	{
		if (freeNodes)
		{
			Node* pNode = table->table[i];
			while (pNode)
			{
				Node* pNodeC = pNode;
				pNode = pNode->next;
				free(pNodeC);
			}
		}
		table->table[i] = NULL;
	}
	table->size = 0;
}

void hash_table_iter_init(HashTable* table, HashTableIter* iter)
{
	HashTableIterPriv* it = (HashTableIterPriv*)iter;
	if (!it)
		return;
	it->table = table;
	it->index = -1;
	it->pNode = NULL;
}

int hash_table_iter_next(HashTableIter* iter, pointer* key, pointer* val)
{
	HashTableIterPriv* it = (HashTableIterPriv*)iter;
	if (!it)
		return FALSE;
	if (!it->pNode)
	{
		if (it->index + 1 == it->table->capacity)
			return FALSE;
		it->index++;
		it->pNode = it->table->table[it->index];
		return hash_table_iter_next(iter, key, val);
	}
	if (key) *key = it->pNode->key;
	if (val) *val = it->pNode->val;
	it->pNode = it->pNode->next;
	return TRUE;
}

