#include "linked_list.h"

linked_list_t *ll_create(unsigned int data_size)
{
    linked_list_t* ll;
 
    ll = malloc(sizeof(*ll));
    ll->head = NULL;
    ll->data_size = data_size;
    ll->size = 0;
 
    return ll;
}

void ll_add_nth_node(linked_list_t* list, unsigned int n, const void* new_data)
{
    ll_node_t *prev;
    ll_node_t *curr;
    ll_node_t *new_node;

    if (!list) {
        return;
    }

    if (n > list->size)
        n = list->size;
    
    if (n < 0)
        return;
    
    new_node = malloc(sizeof(*new_node));
    new_node -> data = malloc(list -> data_size);
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
        return NULL;
    }
 
    if (list->head == NULL) { /* Lista este goala. */
        return NULL;
    }
 
    /* n >= list->size - 1 inseamna eliminarea nodului de la finalul listei. */
    if (n > list->size - 1) {
        n = list->size - 1;
    }
 
    curr = list->head;
    prev = NULL;
    while (n > 0) {
        prev = curr;
        curr = curr->next;
        --n;
    }
 
    if (prev == NULL) {
        /* Adica n == 0. */
        list->head = curr->next;
    } else {
        prev->next = curr->next;
    }
 
    list->size--;
 
    return curr;
}

unsigned int ll_get_size(linked_list_t* list)
{
   if (list == NULL) {
        return -1;
    }
 
    return list->size;
}

void ll_free(linked_list_t** pp_list)
{
    ll_node_t* currNode;
 
    if (pp_list == NULL || *pp_list == NULL) {
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

void ll_print_int(linked_list_t* list)
{
    ll_node_t* curr;
 
    if (list == NULL) {
        return;
    }
 
    curr = list->head;
    while (curr != NULL) {
        printf("%d ", *((int*)curr->data));
        curr = curr->next;
    }
 
    printf("\n");
}

void ll_print_string(linked_list_t *list)
{
	ll_node_t *curr;

	if (!list) {
		return;
	}

	curr = list->head;
	while (curr != NULL) {
		printf("%s ", (char *)curr->data);
		curr = curr->next;
	}

	printf("\n");
}
