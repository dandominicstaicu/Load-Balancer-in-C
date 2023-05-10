// Copyright 2023 <Dan-Dominic Staicu>
// Copyright 2023 SD lab
#ifndef HASH_TABLE_H_
#define HASH_TABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linked_list.h"

#define HMAX 30

typedef struct info_t info_t;
struct info_t {
	void *key;
	void *value;
};

typedef struct hashtable_t hashtable_t;
struct hashtable_t {
	// array of linked lists
	linked_list_t **buckets;
	// total number of nodes existing in all the buckets
	unsigned int size;
	// no buckets
	unsigned int hmax;
	// pointer to the hash value computing function
	unsigned int (*hash_function)(void *);
	// pointer to key comparison function
	int (*compare_function)(void *, void *);
	/// pointer to the free key and value function
	void (*key_val_free_function)(void *);
};

int compare_function_ints(void *a, void *b);

int compare_function_strings(void *a, void *b);

unsigned int hash_function_int(void *a);

unsigned int hash_function_string(void *a);

void key_val_free_function(void *data);

hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void *),
		       int (*compare_function)(void *, void *),
		       void (*key_val_free_function)(void *));

int ht_has_key(hashtable_t *ht, void *key);

void *ht_get(hashtable_t *ht, void *key);

void ht_put(hashtable_t *ht, void *key, unsigned int key_size, void *value,
	    unsigned int value_size);

void ht_remove_entry(hashtable_t *ht, void *key);

void ht_free(hashtable_t *ht);

unsigned int ht_get_size(hashtable_t *ht);

unsigned int ht_get_hmax(hashtable_t *ht);

#endif  // HASH_TABLE_H_
