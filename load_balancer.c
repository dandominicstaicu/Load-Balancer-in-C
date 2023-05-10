// Copyright 2023 <Dan-Dominic Staicu>
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

// initializes a new load balancer and returns a pointer to the new load_bal
load_balancer_t *init_load_balancer() {
	// alloc mem for load balancer
    load_balancer_t *ld_bal = calloc(1, sizeof(load_balancer_t));
	DIE(!ld_bal, "calloc of load balancer failed in init_load_balancer\n");

	// create a ll to hold server_memory_t* items
	ld_bal->ring = ll_create(sizeof(server_memory_t *));

    return ld_bal;
}

// add a new replica of a given server in the ring
void add_one_replica(load_balancer_t *main, int server_id)
{
	// allocate mem for the new server and initialize it
    server_memory_t *srv = init_server_memory();
	srv->hash = hash_function_servers(&server_id);
    srv->id = server_id;

    // interclass the new server in the sorted list of servers
	int count = 0;
    ll_node_t *node = main->ring->head;

    while (node) {
		server_memory_t **ptr = (server_memory_t **)node->data;
		server_memory_t *curr_server = *ptr;

        if (srv->hash < curr_server->hash) {
            ll_add_nth_node(main->ring, count, &srv);
            break;
        }

        node = node->next;
		++count;
    }

	// if not found, add at the end a new server
    if (!node)
        ll_add_nth_node(main->ring, main->ring->size, &srv);

	// check if the number of servers in the hashring is greater than 1 (3 reps)
    // than, rebalance the data
    if (main->ring->size <= MAX_COPY_CNT)
        return;

    node = main->ring->head;

    while (node) {
		server_memory_t **ptr = (server_memory_t **)node->data;
		server_memory_t *curr_server = *ptr;

        if (srv->id == curr_server->id)
            break;

        node = node->next;
    }

    ll_node_t *next = node->next;
    if (!next)
        next = main->ring->head;

    // get the server that comes after the new server in the sorted list
	server_memory_t **ptr = (server_memory_t **)next->data;
	server_memory_t *next_server = *ptr;

    // get the case of server positioning and rebalance acordingly
	// depending on the position of the new server and it's next, move data
	// from the next server to the new server in order to "balance the load"
    if (count == 0) {
		// if the new server is the first, move data from the last server to it
		for (unsigned int i = 0; i < next_server->ht->hmax; ++i) {
            ll_node_t *node = next_server->ht->buckets[i]->head;

		   	// Iterate the ht of the next server to check for key-value pairs
			// that should be moved to the new server
            while (node) {
                char *key = (char *)((info_t *)node->data)->key;
                char *value = (char *)((info_t *)node->data)->value;
                unsigned int hash = hash_function_key(key);

				// the key-value pair should be moved to the new server
                if (next_server->hash < hash || srv->hash >= hash) {
                    server_store(srv, key, value);
                    node = node->next;
                    server_remove(next_server, key);
                    continue;
                }
                node = node->next;
            }
        }
    } else if (next == main->ring->head) {
		// if the new srv is the last one in the ring
		// move data from the first to the last
		for (unsigned int i = 0; i < next_server->ht->hmax; ++i) {
            ll_node_t *node = next_server->ht->buckets[i]->head;

			while (node) {
                char *key = (char *)((info_t *)node->data)->key;
                char *value = (char *)((info_t *)node->data)->value;
                unsigned int hash = hash_function_key(key);

				if (hash <= srv->hash && hash > next_server->hash) {
                    server_store(srv, key, value);
                    node = node->next;
                    server_remove(next_server, key);
                    continue;
                }

                node = node->next;
            }
        }
    } else {
		// the new server is between two servers in the ring
		for (unsigned int i = 0; i < next_server->ht->hmax; ++i) {
            ll_node_t *node = next_server->ht->buckets[i]->head;

		    while (node) {
                char *key = (char *)((info_t *)node->data)->key;
                char *value = (char *)((info_t *)node->data)->value;
                unsigned int hash = hash_function_key(key);

				if (srv->hash >= hash) {
                    server_store(srv, key, value);
                    node = node->next;
                    server_remove(next_server, key);
                    continue;
                }

                node = node->next;
            }
        }
    }
}

// add a new server by adding multiple replicas of the server
void loader_add_server(load_balancer_t* main, int server_id) {
	for (int i = 0; i < MAX_COPY_CNT; ++i)
        add_one_replica(main, i * POW5 + server_id);
}

// remove a replica of the server from the hash ring
void remove_replica(load_balancer_t *main, int server_id)
{
	ll_node_t *node = main->ring->head;
	int cnt = 0;

	// iterate through all the servers to find the node of the server to rmv
	while (node) {
		server_memory_t **ptr = (server_memory_t **)node->data;
		server_memory_t *server = *ptr;

		// stop when the id is matched
		if (server->id == server_id)
			break;

		++cnt;
		node = node->next;
	}

	// get the server from data
	server_memory_t **ptr = (server_memory_t **)node->data;
	server_memory_t *server = *ptr;

	// get the next server
	node = node->next;
	// if last, the next is first
	if (!node)
		node = main->ring->head;

	ptr = (server_memory_t **)node->data;
	server_memory_t *next_srv = *ptr;

	// move all data from the server to remove to the next server
	server_empty(server, next_srv);

	// remove the server node from the hashring and free the server memory
	ll_node_t *rmv = ll_remove_nth_node(main->ring, cnt);

	free_server_memory(server);
	free(rmv->data);
	free(rmv);
}

// remove a server from the load balancer by removing all of its replicas
void loader_remove_server(load_balancer_t *main, int server_id) {
    for (int i = 0; i < MAX_COPY_CNT; ++i)
		remove_replica(main, i * POW5 + server_id);
}

// stores a key-value pair in the correct server
// also returns the server id where the key-value pair is stored
void loader_store(load_balancer_t *main, char *key, char *value,
				  int *server_id) {
	// check if there is at least one server available
	if (!main->ring->head) {
		ERROR("no server available\n");
		return;
	}

	ll_node_t *node = main->ring->head;

	// create the hash of the key
	unsigned int key_hash = hash_function_key(key);

	// iterate through all the servers in order to find an appropriate one
	while (node) {
		server_memory_t **ptr = (server_memory_t **)node->data;
		server_memory_t *srv = *ptr;

		// if found, store the key-value pair in the server and return its id
		if (srv->hash >= key_hash) {
			server_store(srv, key, value);
			*server_id = srv->id % POW5;
			return;
		}

		node = node->next;
	}

	// If the hash is greater than the highest server hash, store the pair in
	// the first server of the hashring

	node = main->ring->head;

	server_memory_t **ptr = (server_memory_t **)node->data;
	server_memory_t *srv = *ptr;

	server_store(srv, key, value);
	*server_id = srv->id % POW5;
}

// retrieves a value from a server by its key
char *loader_retrieve(load_balancer_t *main, char *key, int *server_id) {
    if (!main->ring->head) {
		ERROR("no server found");
		return NULL;
	}

	// get the hash of the key
	unsigned int key_hash = hash_function_key(key);


	// iterate through the servers in the hashring
	ll_node_t *node = main->ring->head;

	while (node) {
		server_memory_t **ptr = (server_memory_t **)node->data;
		server_memory_t *srv = *ptr;

		// if the key hash is less than or equal to this server's hash,
		// use this server to retrieve the value
		if (srv->hash >= key_hash) {
			*server_id = srv->id % POW5;

			char *retrieved_data = server_retrieve(srv, key);
			return retrieved_data;
		}

		node = node->next;
	}

	// if no server was found, use the first from the hashring
    node = main->ring->head;

	server_memory_t **ptr = (server_memory_t **)node->data;
	server_memory_t *srv = *ptr;

	*server_id = srv->id % POW5;

	char *retrieved_data = server_retrieve(srv, key);
	return retrieved_data;
}

// frees all the memory used by the load balancer and it's servers
void free_load_balancer(load_balancer_t *main) {
    if (!main) {
		ERROR("main is NULL");
		return;
	}

	if (!main->ring) {
		ERROR("ring is NULL");

		free(main);
		return;
	}

	ll_node_t *node = main->ring->head;

	// iterate through all the servers and free them one by one
	while (node) {
		server_memory_t **ptr = (server_memory_t **)node->data;
		server_memory_t *srv = *ptr;
		free_server_memory(srv);

		node = node->next;
	}

	// free the pointers
	ll_free(&main->ring);
	free(main);
}
