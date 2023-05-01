/*  Copyright 2021 Buzea Alexandru-Mihai-Iulian 311CAb
    and Data Structures Team (hash functions and skel)  */

#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"

/*  Our structure for load balancer: it contains the hashring
 *  implementation (a linked list) and two pointers to hash functions
 */

struct load_balancer {
	linked_list_t *hashring;
    unsigned int (* hash_server)(void *);
    unsigned int (* hash_key)(void *);
};

unsigned int hash_function_servers(void *a) {
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

unsigned int hash_function_key(void *a) {
    unsigned char *puchar_a = (unsigned char *) a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}


load_balancer* init_load_balancer() {
    /*  Allocation of memory for the load balancer  */
	load_balancer *load = (load_balancer *)malloc(sizeof(load_balancer));
    DIE(!load, "Malloc failed\n");

    /*  Initialization of the load balancer fields  */
    load->hashring = ll_create(sizeof(server_memory *));
    load->hash_server = hash_function_servers;
    load->hash_key = hash_function_key;

    return load;
}

void loader_store(load_balancer* main, char* key, char* value, int* server_id)
{
    if (!main->hashring->head) {
        fprintf(stderr, "No existing server!\n");
        return;
    }

    ll_node_t *it = main->hashring->head;

    unsigned int key_hash = main->hash_key(key);

    /*  finding the first server with the hash greater than key hash  */

    while (it != NULL) {
        server_memory *server = *(server_memory **)(it->data);

        if (key_hash <= server->hash) {
            server_store(server, key, value);
            *server_id = server->id % NUM_SERVERS;
            return;
        }

        it = it->next;
    }

    /*  if the key hash is greater than that of any server, put the key
     *  in the first server from the list  */

    it = main->hashring->head;
    server_memory *server = *(server_memory **)(it->data);
    server_store(server, key, value);
    *server_id = server->id % NUM_SERVERS;
}

char* loader_retrieve(load_balancer* main, char* key, int* server_id) {
	unsigned int hash_key = main->hash_key(key);

    ll_node_t *it = main->hashring->head;

    if (!it) {
        fprintf(stderr, "No existing server !\n");
        return NULL;
    }

    /*  if we find the server we are searching for, retrieve the key  */

    while (it != NULL) {
        server_memory *server = *(server_memory **)(it->data);

        if (hash_key <= server->hash) {
            *server_id = server->id % NUM_SERVERS;
            return server_retrieve(server, key);
        }

        it = it->next;
    }
    /* if we haven't found the server with the hash greater than our
     * key hash, it means that the key can be stored on the first server */

    it = main->hashring->head;

    server_memory *server = *(server_memory **)(it->data);
    *server_id = server->id % NUM_SERVERS;

    return server_retrieve(server, key);
}

void loader_add_copy(load_balancer *main, int server_id)
{
    server_memory *server = init_server_memory();

    server->id = server_id;
    server->hash = main->hash_server(&server_id);

    int count = 0;
    ll_node_t *it = main->hashring->head;

    /* adding server in the sorted list of servers (by hash) */

    while (it != NULL) {
        server_memory *curr_server = *(server_memory **)(it->data);

        if (server->hash < curr_server->hash) {
            ll_add_nth_node(main->hashring, count, &server);
            break;
        }

        ++count;
        it = it->next;
    }

    if (it == NULL)
        ll_add_nth_node(main->hashring, main->hashring->size, &server);

    if (main->hashring->size <= NUM_COPY)
        return;

    it = main->hashring->head;

    while (it != NULL) {
        server_memory *curr_server = *(server_memory **)(it->data);

        if (server->id == curr_server->id)
            break;

        it = it->next;
    }

    ll_node_t *next = it->next;

    if (next == NULL)
        next = main->hashring->head;

    /* get the pointer to the next server */

    server_memory *next_server = *(server_memory **)(next->data);

    int type;

    /* identifying the type of the server position */

    if (count == 0) {
        type = 1;
    } else if (next == main->hashring->head) {
        type = 2;
    } else {
        type = 3;
    }

    server_rebalance(server, next_server, type, main->hash_key);
}

void loader_add_server(load_balancer* main, int server_id) {
	for (int i = 0; i < NUM_COPY; ++i)
        loader_add_copy(main, i * NUM_SERVERS + server_id);
}

void loader_remove_copy(load_balancer *main, int server_id)
{
    ll_node_t *it = main->hashring->head;
    int count = 0;

    /* serching for the server by server_id */

    while (it != NULL) {
        server_memory *server = *(server_memory **)(it->data);
        if (server->id == server_id)
            break;

        ++count;
        it = it->next;
    }

    server_memory *server = *(server_memory **)(it->data);

    it = it->next;
    if (it == NULL)
        it = main->hashring->head;

    server_memory *next_server = *(server_memory **)(it->data);

    server_empty(server, next_server);

    /* after we moved the keys, we can remove the server from the list */

    ll_node_t *to_remove = ll_remove_nth_node(main->hashring, count);

    free_server_memory(server);
    free(to_remove->data);
    free(to_remove);
}

void loader_remove_server(load_balancer* main, int server_id) {
	for (int i = 0; i < NUM_COPY; ++i)
        loader_remove_copy(main, i * NUM_SERVERS + server_id);
}

void free_load_balancer(load_balancer* main) {
    if (!main)
        return;

    if (!main->hashring) {
        free(main);
        return;
    }

    ll_node_t *it = main->hashring->head;

    /* freeing all the allocated memory for servers */

    while (it != NULL) {
        server_memory *server = *(server_memory **)(it->data);
        free_server_memory(server);

        it = it->next;
    }

    /* freeing the hashring and the load balancer itself */

    ll_free(&main->hashring);
    free(main);
}
