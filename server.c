// Copyright 2023 <Dan-Dominic Staicu>
#include "server.h"

// create a new server and alloc memory for it
server_memory_t *init_server_memory()
{
	server_memory_t *srv = calloc(1, sizeof(server_memory_t));
	DIE(!srv, "srv calloc failed in init_server_memory()\n");

	srv->hash = 0;
	srv->id = 0;

	srv->ht = ht_create(HMAX, hash_function_string, compare_function_strings,
						key_val_free_function);

	return srv;
}

// add a new item on a server in the server's ht
void server_store(server_memory_t *server, char *key, char *value) {
	if (!server) {
		ERROR("Server was not alloc'd\n");
		return;
	}

	unsigned int key_len = strlen(key) + 1;
	unsigned int val_len = strlen(value) + 1;
	ht_put(server->ht, key, key_len, value, val_len);
}

// get an item from server memory's ht
char *server_retrieve(server_memory_t *server, char *key) {
	if (!server) {
		ERROR("server was not created\n");
		return NULL;
	}

	char *retrieved_data = ht_get(server->ht, key);

	return retrieved_data;
}

// remove an item from server's memory
void server_remove(server_memory_t *server, char *key) {
	if (!server) {
		ERROR("server was not created\n");
		return;
	}

	ht_remove_entry(server->ht, key);
}

// clear all the items from servers memory hashtable
void free_server_memory(server_memory_t *server) {
	if (!server || !server->ht) {
		if (!server->ht)
			free(server);

		ERROR("server was not created\n");
		return;
	}

	ht_free(server->ht);

	free(server);
}

// move items from source server to a new server (dest server)
void server_empty(server_memory_t *src_srv, server_memory_t *dest_srv)
{
	if (!src_srv) {
		ERROR("src_server does not exist or was not alloc'd\n");
		return;
	}

	if (!dest_srv) {
		ERROR("dest_server does not exist or was not alloc'd\n");
		return;
	}

    for (unsigned int i = 0; i < src_srv->ht->hmax; ++i) {
        ll_node_t *node = src_srv->ht->buckets[i]->head;

        while (node) {
			char *key = ((info_t *)node->data)->key;
			char *value = ((info_t *)node->data)->value;

			server_store(dest_srv, key, value);

			node = node->next;

            server_remove(src_srv, key);
        }
    }
}

