/* Copyright 2023 <> */
#include <stdlib.h>
#include <string.h>

#include "server.h"

server_memory_t *init_server_memory()
{
	server_memory_t *srv = calloc(1, sizeof(server_memory_t));
	DIE(!srv, "srv calloc failed in init_server_memory()\n");

	srv->hash = 0;
	srv->id = 0;
	
	srv->ht = ht_create(HMAX, hash_function_string, compare_function_strings, key_val_free_function);

	return srv;
}

void server_store(server_memory_t *server, char *key, char *value) {
	if (!server)
		return;

	unsigned int key_len = strlen(key) + 1;
	unsigned int val_len = strlen(value) + 1;
	ht_put(server->ht, key, key_len, value, val_len);
}

char *server_retrieve(server_memory_t *server, char *key) {
	if (!server)
		return NULL;

	char *retrieved_data = ht_get(server->ht, key);
	
	return retrieved_data;
}

void server_remove(server_memory_t *server, char *key) {
	if (!server)
		return;

	ht_remove_entry(server->ht, key);
}

void free_server_memory(server_memory_t *server) {
	if (!server || !server->ht) {
		if (!server->ht)
			free(server);
		
		return;
	}

	ht_free(server->ht);

	free(server);
}
