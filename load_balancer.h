/* Copyright 2023 <> */
#ifndef LOAD_BALANCER_H_
#define LOAD_BALANCER_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "server.h"
#include "linked_list.h"
#include "hash_table.h"

#define POW5 100000
#define MAX_COPY_CNT 3

struct load_balancer_t;
typedef struct load_balancer_t load_balancer_t;

struct load_balancer_t {
    linked_list_t *ring;
};

/**
 * init_load_balancer() - initializes the memory for a new load balancer and its fields and
 *                        returns a pointer to it
 *
 * Return: pointer to the load balancer struct
 */
load_balancer_t *init_load_balancer();

/**
 * free_load_balancer() - frees the memory of every field that is related to the
 * load balancer (servers, hashring)
 *
 * @arg1: Load balancer to free
 */
void free_load_balancer(load_balancer_t *main);

/**
 * load_store() - Stores the key-value pair inside the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: Value represented as a string.
 * @arg4: This function will RETURN via this parameter
 *        the server ID which stores the object.
 *
 * The load balancer will use Consistent Hashing to distribute the
 * load across the servers. The chosen server ID will be returned
 * using the last parameter.
 *
 * Hint:
 * Search the hashring associated to the load balancer to find the server where the entry
 * should be stored and call the function to store the entry on the respective server.
 *
 */
void loader_store(load_balancer_t *main, char *key, char *value,
				  int *server_id);

/**
 * load_retrieve() - Gets a value associated with the key.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: This function will RETURN the server ID
          which stores the value via this parameter.
 *
 * The load balancer will search for the server which should posess the
 * value associated to the key. The server will return NULL in case
 * the key does NOT exist in the system.
 *
 * Hint:
 * Search the hashring associated to the load balancer to find the server where the entry
 * should be stored and call the function to store the entry on the respective server.
 */
char *loader_retrieve(load_balancer_t *main, char *key, int *server_id);

/**
 * load_add_server() - Adds a new server to the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the new server.
 *
 * The load balancer will generate 3 replica labels and it will
 * place them inside the hash ring. The neighbor servers will
 * distribute some the objects to the added server.
 *
 * Hint:
 * Resize the servers array to add a new one.
 * Add each label in the hashring in its appropiate position.
 * Do not forget to resize the hashring and redistribute the objects
 * after each label add (the operations will be done 3 times, for each replica).
 */
void loader_add_server(load_balancer_t *main, int server_id);

/**
 * load_remove_server() - Removes a specific server from the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the removed server.
 *
 * The load balancer will distribute ALL objects stored on the
 * removed server and will delete ALL replicas from the hash ring.
 *
 */
void loader_remove_server(load_balancer_t *main, int server_id);

unsigned int hash_function_servers(void *a);

unsigned int hash_function_key(void *a);

void add_one_replica(load_balancer_t *main, int server_id);

void rebalance(server_memory_t *src_srv, server_memory_t *dest_srv);

void remove_replica(load_balancer_t *main, int server_id);

void server_rebalance(server_memory_t *server, server_memory_t *next_server,
                      int type, unsigned int (* hash_key)(void *));


void loader_add_copy(load_balancer_t *main, int server_id);



#endif /* LOAD_BALANCER_H_ */
