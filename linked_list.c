// Copyright 2023 <Dan-Dominic Staicu>
#include "linked_list.h"

// create a new list and init it
linked_list_t *ll_create(unsigned int data_size)
{
    linked_list_t *ll = malloc(sizeof(linked_list_t));
    ll->head = NULL;
    ll->data_size = data_size;
    ll->size = 0;

    return ll;
}

// add a new node in the ll at any given position
void ll_add_nth_node(linked_list_t* list, unsigned int n, const void* new_data)
{
    ll_node_t *prev;
    ll_node_t *curr;
    ll_node_t *new_node;

    if (!list) {
		ERROR("list is NULL");
		return;
	}

    if (n > list->size)
        n = list->size;

    new_node = malloc(sizeof(ll_node_t));
	DIE(!new_node, "malloc of new node failed\n");

    new_node -> data = malloc(list -> data_size);
	DIE(!new_node->data, "malloc of data of the new node failed\n");

    memcpy(new_node->data, new_data, list->data_size);

    prev = NULL;
    curr = list -> head;
    while (n) {
        prev = curr;
        curr = curr -> next;
        n--;
    }

    new_node -> next = curr;
    if (prev == NULL)
        list -> head = new_node;
    else
        prev -> next = new_node;

    list -> size++;
}

ll_node_t *ll_remove_nth_node(linked_list_t* list, unsigned int n)
{
    ll_node_t *prev, *curr;

    if (list == NULL) {
		ERROR("list does not exist\n");
        return NULL;
    }

	// list is empty
    if (list->head == NULL) {
		ERROR("list is empty error\n");
        return NULL;
    }

	// remove the last node if the value is greater than the size
    if (n > list->size - 1) {
        n = list->size - 1;
    }

	// get the previous node
    curr = list->head;
    prev = NULL;
    while (n > 0) {
        prev = curr;
        curr = curr->next;
        --n;
    }

	// if n == 0, so no previous
    if (!prev) {
        list->head = curr->next;
    } else {
        prev->next = curr->next;
    }

    list->size--;

    return curr;
}

// return the size of the list
unsigned int ll_get_size(linked_list_t* list)
{
	if (!list) {
		ERROR("list was not init");
        return -1;
    }

    return list->size;
}

// free all the nodes of the list
void ll_free(linked_list_t **pp_list)
{
    ll_node_t *currNode;

	// if the list was not init
    if (!pp_list || *pp_list == NULL) {
		ERROR("list does not exist\n");
        return;
    }

    while (ll_get_size(*pp_list) > 0) {
        currNode = ll_remove_nth_node(*pp_list, 0);
        free(currNode->data);
        free(currNode);
    }

    free(*pp_list);
    *pp_list = NULL;
}

// print the int elements of the list
void ll_print_int(linked_list_t *list)
{
    ll_node_t* curr;

    if (!list) {
		ERROR("list does not exist\n");
        return;
    }

    curr = list->head;
    while (curr != NULL) {
        printf("%d ", *((int*)curr->data));
        curr = curr->next;
    }

    printf("\n");
}

// print the elements as strings
void ll_print_string(linked_list_t *list)
{
	ll_node_t *curr;

	if (!list) {
		ERROR("list does not exist\n");
		return;
	}

	curr = list->head;
	while (curr != NULL) {
		printf("%s ", (char *)curr->data);
		curr = curr->next;
	}

	printf("\n");
}
