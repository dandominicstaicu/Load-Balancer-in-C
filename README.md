# LOAD BALANCER

## License

This Homework was written by **Dan-Dominic Staicu** 311CAb

## Homework quote

> "Franklin's Rule: Blessed is the end user who expects nothing, for he/she will not be disappointed."

<https://www.angelo.edu/faculty/kboudrea/cheap/cheap3_murphy.htm#Computers>

## Description

In this homework the target was to implement a Load Balancer using *Consistent Hasing* in order to simulate the way several items from a e-store's database could be held on multiple servers being split in equal/balanced loads; 

Consistent Hasing is a distributed hasing method used so that when rescaling the table only n / m keys will be remaped (n no keys, m no servers);

An "imaginary Hash Ring" is used so that a server responsible for storing an object would be the closest clockwise on the hashring;

## Server Implementation

In order to create the servers that will be saved on the Hashring, I used the data structure called server_memory_t where are stored a hashtable (key-value pairs of items), it's id and it's hash;

## Hash Ring/Load Balancer Implementation

The Load Balancer was created using a linked list which contains pointers to server_memory_t** elements; it has it's nodes sorted increasingly according to server hashes;

Each server has 3 replicas on the Hash Ring in order to ensure that objects are uniformly spread around the ring and servers, as requested;

## Server operations

1. init_server_memory() - allocs memory for a new server replica (ht, id, hash)

2. server_store() - adds a new item in the given server's hashtable

3. server_retrieve() - get a rquested element from given server's hashtable

4. server_remove() - remove a given item from a given server's hashtable

5. free_server_memory() - clears all item's from server's hashtable and frees the pointer of the ht

## Load Balancer operations

1. loader_store() - stores key-value pair on the hash ring on the appropriate server; it iterates through all the servers stored on the hash ring and stops when the key hash is greater (or equal) to the current server hash; it is posible, because the servers are sorted inside the ring;

2. loader_retrieve() - search for the server where the hash is stored and call server_retireve on the found server; if the server was not found, this means it has to save it on the first server (according to the 360 degrees ring)

3. loader_add_server() - add a server as 3 replicas (as described in the requirements) by calling the add_one_replica function 3 times (for each id, starting with 1, 2 or 3); this function creates a new server_memory_t* object and interclasses it on the hash ring at the appropriate place so that the linked list is always sorted; this process is explaned line by line with comments in load_balancer.c

4. loader_remove_server() - this function removes the server with all it's replicas from the ring; when the given server id matches with one from the ring, it moves all the data from the current server to the next one in order to keep all the items from that replica server and also keep the ring balanced; all the alloc'd memory of each replica is free'd

## Comments

During this homework I got falimiar with using hash tables, but also using 2 data structures simultaniously, according to the requirements of the given task, where they are efficient;

Also, during this homework I learned about a new method, Consistent Hashing because I had to research it in order to get aa better understanding about it's implementation

## Bibliography

I used hashtable and linked list implementations from Data Structure labs where I added my own comments in order to provide a better understanding over these structures