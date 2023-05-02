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

// void add_one_replica(load_balancer_t *main, int server_id)
// {
// 	server_memory_t *srv = init_server_memory();

// 	srv->id = server_id;
// 	srv->ht = hash_function_servers(&server_id);

// 	int cnt = 0;
// 	ll_node_t *node = main->ring->head;

// 	while(node) {
// 		server_memory_t *loc_srv = *(server_memory_t **)(node->data);

// 		if (srv->hash < loc_srv->hash) {
// 			ll_add_nth_node(main->ring, cnt, &srv);
// 			break;
// 		}

// 		++cnt;
// 		node = node->next;
// 	}

// 	if (!node)
// 		ll_add_nth_node(main->ring, main->ring->size, &srv);

	

// }

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
			server_memory_t *next_srv = *(server_memory_t **)(node->next->data);

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

//for (int i = 0; i < main->ring->size; ++i) {
	
	while(node) {
		if (loc_srv->id == server_id) {
			//rebalance();
			//TODO rebalance
			break;
		}

		node = node->next;
	}
}

void loader_add_server(load_balancer_t *main, int server_id) {
    
}

void loader_remove_server(load_balancer_t *main, int server_id) {
    /* TODO 3 */
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
    /* TODO 6 */
}
