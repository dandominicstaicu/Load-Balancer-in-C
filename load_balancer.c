/* Copyright 2023 <> */
#include "load_balancer.h"

unsigned int hash_function_servers(void *a) {
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

unsigned int hash_function_key(void *a) {
    unsigned char *puchar_a = (unsigned char *)a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

load_balancer_t *init_load_balancer() {
    load_balancer_t *ld_bal = calloc(1, sizeof(load_balancer_t));
	DIE(!ld_bal, "calloc of load balancer failed in init_load_balancer\n");

	ld_bal->ring = ll_create(sizeof(server_memory_t *));
	ld_bal->server_hash = hash_function_servers;
	ld_bal->key_hash = hash_function_key;

    return ld_bal;
}

void add_one_replica(load_balancer_t *main, int server_id)// server_memory_t *new_srv_mem)
{
	server_memory_t *new_server = init_server_memory();
	new_server->hash = hash_function_servers(&server_id);
	new_server->id = server_id;

	if (!main->ring->size) {
		ll_add_nth_node(main->ring, 0, &new_server); // TODO MACRO 0
	} else {
		ll_node_t *node = main->ring->head;

		for (int i = 0; i < main->ring->size; ++i) {
			server_memory_t *loc_srv = *(server_memory_t **)(node->data);

			server_memory_t *next_srv = NULL;
			//TODO check if next exists, else next is first
			if (node->next)
				next_srv = *(server_memory_t **)(node->next->data);
			else
				next_srv = *(server_memory_t **)(main->ring->head->data);

			if (new_server->hash > loc_srv->hash && new_server->hash < next_srv->hash) {
				ll_add_nth_node(main->ring, i + 1, &new_server);
				break;				
			} else if (new_server->hash > loc_srv->hash && i == main->ring->size - 1) {
				ll_add_nth_node(main->ring, main->ring->size, &new_server);
				break;
			} else if (new_server->hash < loc_srv->hash) {
				ll_add_nth_node(main->ring, 0, &new_server); //TODO MACRO 0
				break;
			}

			node = node->next;
		}
	}

	ll_node_t *node = main->ring->head;
	server_memory_t *loc_srv = *(server_memory_t **)(node->data);
	server_memory_t *next_srv = NULL;


	while(node) {
		if (loc_srv->id == server_id) {
			//rebalance();
			//TODO rebalance
			break;
		}

		node = node->next;
	}
}

void rebalance(server_memory_t *src_srv, server_memory_t *dest_srv)
{
	if (!src_srv || !dest_srv) {
		//TODO macro error
		return;
	}

	for (int i = 0; i < src_srv->ht->hmax; ++i) {
		ll_node_t *node = src_srv->ht->buckets[i]->head;
		ll_node_t *current = NULL;

		int size = src_srv->ht->buckets[i]->size;
		for (int j = 0; j < size; ++j) {
			current = node;
			node = node->next;

			char *key = ((info *)current->data)->key;
			char *value = ((info *)current->data)->value;
			unsigned int key_len = strlen(key) + 1;
			unsigned int value_len = strlen(value) + 1;

			if (dest_srv->hash >= hash_function_key(key)) {
				ht_put(dest_srv->ht, key, key_len, value, value_len);
				ht_remove_entry(src_srv, key);
			}
		}
	}
}

void loader_add_server(load_balancer_t *main, int server_id) {
    for (int i = 0; i <= MAX_COPY_CNT; ++i)
		add_one_replica(main, i * POW5 + server_id);
}

void remove_replica(load_balancer_t *main, int server_id)
{
	ll_node_t *node = main->ring->head;
	int cnt = 0;

	while (node) {
		server_memory_t *server = *(server_memory_t **)(node->data);
		if (server->id == server_id)
			break;

		++cnt;
		node = node->next;
	}

	server_memory_t *server = *(server_memory_t **)(node->data);
	
	node = node->next;
	if (!node)
		node = main->ring->head;

	server_memory_t *next_srv = *(server_memory_t **)(node->data);

	server_empty(server, next_srv);

	ll_node_t *rmv = ll_remove_nth_node(main->ring, cnt);

	free_server_memory(server);
	free(rmv->data);
	free(rmv);
}

void loader_remove_server(load_balancer_t *main, int server_id) {
    for (int i = 0; i < MAX_COPY_CNT; ++i)
		remove_replica(main, server_id);
}

void loader_store(load_balancer_t *main, char *key, char *value, int *server_id) {
    if (!main->ring->head) {
		NO_SERVER;
		return;
	}

	ll_node_t *node = main->ring->head;

	unsigned int key_hash = hash_function_key(key);

	while (node) {
		server_memory_t *srv = *(server_memory_t **)(node->data);

		if (key_hash <= srv->hash) {
			server_store(srv, key, value);
			*server_id = srv->id % POW5;
			return;
		}

		node = node->next;
	}

	node = main->ring->head;
	server_memory_t *srv = *(server_memory_t **)(node->data);

	server_store(srv, key, value);
	*server_id = srv->id % POW5;
}

char *loader_retrieve(load_balancer_t *main, char *key, int *server_id) {
    if (!main->ring->head) {
		NO_SERVER;
		return;
	}
	
	unsigned int key_hash = hash_function_key(key);

	ll_node_t *node = main->ring->head;

	while(node) {
		server_memory_t *srv = *(server_memory_t **)node->data;

		if (key_hash <= srv->hash) {
			*server_id = srv->id % POW5;
			return server_retrieve(srv, key);
		}

		node = node->next;
	}


    node = main->ring->head;
	server_memory_t *srv = *(server_memory_t **)(node->data);

	*server_id = srv->id % POW5;

	return server_retrieve(srv, key);
}

void free_load_balancer(load_balancer_t *main) {
    if (!main)
	//TODO macro error
		return;

	if (!main->ring)
	//TODO macro error
		return;

	ll_node_t *node = main->ring->head;

	while (node) {
		server_memory_t *srv = *(server_memory_t **)(node->data);
		free_server_memory(srv);

		node = node->next;
	}

	ll_free(&main->ring);
	free(main);
}
