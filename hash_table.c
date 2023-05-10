// Copyright 2023 <Dan-Dominic Staicu>
#include "hash_table.h"

// functions for key comparison
int compare_function_ints(void *a, void *b)
{
	int int_a = *((int *)a);
	int int_b = *((int *)b);

	if (int_a == int_b) {
		return 0;
	} else if (int_a < int_b) {
		return -1;
	} else {
		return 1;
	}
}

int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

// hashing functions
unsigned int hash_function_int(void *a)
{
	/*
	 * Credits: https://stackoverflow.com/a/12996028/7883884
	 */
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int hash_function_string(void *a)
{
	/*
	 * Credits: http://www.cse.yorku.ca/~oz/hash.html
	 */
	unsigned char *puchar_a = (unsigned char *)a;
	unsigned long hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c; /* hash * 33 + c */

	return hash;
}

// used to free the mem of a pair (key, value) from the hashtable
void key_val_free_function(void *data)
{
	info_t *data_info = (info_t *)data;
	free(data_info->key);
	free(data_info->value);
}

// used to init a new hashtable, after it was alloc'd
hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void *),
		       int (*compare_function)(void *, void *),
		       void (*key_val_free_function)(void *))
{
	if (!hash_function || !compare_function) {
		return NULL;
	}

	hashtable_t *map = malloc(sizeof(hashtable_t));

	map->size = 0;
	map->hmax = hmax;
	map->hash_function = hash_function;
	map->compare_function = compare_function;
	map->key_val_free_function = key_val_free_function;

	map->buckets = malloc(map->hmax * sizeof(linked_list_t *));
	for (unsigned int i = 0; i < map->hmax; ++i) {
		map->buckets[i] = ll_create(sizeof(info_t));
	}

	return map;
}

// returns 1 if the key is in the ht, else return 0
int ht_has_key(hashtable_t *ht, void *key)
{
	if (!ht || !key) {
		return -1;
	}

	int hash_index = ht->hash_function(key) % ht->hmax;
	ll_node_t *node = ht->buckets[hash_index]->head;

	while (node != NULL) {
		info_t *data_info = (info_t *)node->data;
		if (!ht->compare_function(data_info->key, key)) {
			return 1;
		}
		node = node->next;
	}

	return 0;
}

// get the value of the given key from the ht
void *ht_get(hashtable_t *ht, void *key)
{
	if (!ht || !key || ht_has_key(ht, key) != 1) {
		return NULL;
	}

	int hash_index = ht->hash_function(key) % ht->hmax;
	ll_node_t *node = ht->buckets[hash_index]->head;

	while (node != NULL) {
		info_t *data_info = (info_t *)node->data;
		if (!ht->compare_function(data_info->key, key)) {
			return data_info->value;
		}
		node = node->next;
	}

	return NULL;
}

// insert or update a key-value pair into the hash table
// used to determine the index of the hash table where the pair will be stored
void ht_put(hashtable_t *ht, void *key, unsigned int key_size, void *value,
	    unsigned int value_size)
{
	if (!ht || !key || !value) {
		return;
	}

	int hash_index = ht->hash_function(key) % ht->hmax;

	if (ht_has_key(ht, key) == 1) {
		ll_node_t *node = ht->buckets[hash_index]->head;
		while (node != NULL) {
			info_t *data_info = node->data;

			if (!ht->compare_function(data_info->key, key)) {
				free(data_info->value);

				data_info->value = malloc(value_size);

				memcpy(data_info->value, value, value_size);
				return;
			}

			node = node->next;
		}
	}

	info_t *data_info = malloc(sizeof(info_t));

	data_info->key = malloc(key_size);
	data_info->value = malloc(value_size);

	memcpy(data_info->key, key, key_size);
	memcpy(data_info->value, value, value_size);

	// add every new node on the first position on the list
	ll_add_nth_node(ht->buckets[hash_index], 0, data_info); 	ht->size++;

	free(data_info);
}

// eliminate from the hashtable the entry linked to the given key
void ht_remove_entry(hashtable_t *ht, void *key)
{
	if (!ht || !key || ht_has_key(ht, key) != 1) {
		return;
	}

	int hash_index = ht->hash_function(key) % ht->hmax;
	ll_node_t *node = ht->buckets[hash_index]->head;

	unsigned int node_nr = 0;

	while (node != NULL) {
		info_t *data_info = (info_t *)node->data;

		if (!ht->compare_function(data_info->key, key)) {
			ht->key_val_free_function(data_info);
			// free memory where the data points
			free(data_info);

			// get the node that needs to be freed
			ll_node_t *deleted_node = ll_remove_nth_node(
			    ht->buckets[hash_index], node_nr);
			// free memory where the removed node points
			free(deleted_node);

			ht->size--;
			return;
		}

		node = node->next;
		node_nr++;
	}
}


// free all the memory used by entries in the ht
// and the structure used lastly
void ht_free(hashtable_t *ht)
{
	if (!ht) {
		return;
	}

	for (unsigned int i = 0; i < ht->hmax; ++i) {
		ll_node_t *node = ht->buckets[i]->head;

		while (node != NULL) {
			ht->key_val_free_function(node->data);
			node = node->next;
		}

		// free the memory where the list points and the memory
		// where the nodes point
		ll_free(&ht->buckets[i]);
	}

	// free the memory where the double pointer buckets points, so we have
	// 0 lists in the array
	free(ht->buckets);

	// free the memory where ht points so free the ht
	free(ht);
}

// return the size of the hashtable
unsigned int ht_get_size(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->size;
}

// get the number of buckets
unsigned int ht_get_hmax(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->hmax;
}
