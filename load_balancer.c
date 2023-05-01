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

// void add_

void loader_add_server(load_balancer_t *main, int server_id) {
    /* TODO 2 */
}

void loader_remove_server(load_balancer_t *main, int server_id) {
    /* TODO 3 */
}

void loader_store(load_balancer_t *main, char *key, char *value, int *server_id) {
    /* TODO 4 */
}

char *loader_retrieve(load_balancer_t *main, char *key, int *server_id) {
    /* TODO 5 */
    return NULL;
}

void free_load_balancer(load_balancer_t *main) {
    /* TODO 6 */
}
