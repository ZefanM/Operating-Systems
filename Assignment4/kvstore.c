#include <semaphore.h>
#include <assert.h>
#include <stdbool.h>
#include "server_utils.h"
#include "common.h"
#include "request_dispatcher.h"
#include "hash.h"
#include "kvstore.h"
#include "pthread.h"
// DO NOT MODIFY THIS.
// ./check.py assumes the hashtable has 256 buckets.
// You should initialize your hashtable with this capacity.
#define HT_CAPACITY 256
unsigned int get_index(char* key, unsigned int size){
    return hash(key) % size;
}

int set_request(int socket, struct request *request)
{
    size_t len = 0;
    size_t expected_len = request->msg_len;
    // 1. Lock the hashtable entry. Create it if the key is not in the store.
 
    int index = get_index(request->key,ht->capacity);
    // char* buf = malloc(expected_len);
    hash_item_t *curr_entry = ht->items[index];
    while (curr_entry != NULL) {
        if (strcmp(request->key,curr_entry->key) == 0) {
            free(curr_entry->value);
            // curr_entry->value = buf;
            break;
            // memcpy(curr_entry->value,buf,request->msg_len);
            // memcpy(curr_entry->value_size, expected_len, expected_len);
        }
        curr_entry = curr_entry->next;
    }
    if (curr_entry == NULL) {//if all entries are null just append a new entry(key,value)
        curr_entry = (struct hash_item_t*) malloc(sizeof(struct hash_item_t));
        // curr_entry->value = buf;
    }
    // fprintf(stderr, "REQ-KEY:%s", request->key);
    // fprintf(stderr, "CURR-ENTRY-KEY:%s", curr_entry->key);
    
    int readSize;
    curr_entry->value = malloc(request->msg_len);
    while (len < expected_len) {    
        readSize = read_payload(socket,request,expected_len,curr_entry->value);
        len += readSize;
        curr_entry->value_size = len;
        // 2. Read the payload from the socket
        // Note: Clients may send a partial chunk of the payload so you should not wait
        // for the full data to be available before write in the hashtable entry.
        // 3. Write the partial payload on the entry
    }
    // buf = buf - len;

    // 4. Unlock the entry in the store to finalize the insertion.
    // This allow other threads to read the entry.
    // It checks if the payload has been fully received .
    // It also reads the last char of the request which should be '\n'
    check_payload(socket, request, expected_len);
    // send_response(socket, OK, request->msg_len,ht->items[index]->value);
    send_response(socket, OK,0,NULL);
    // Optionally you can close the connection
    // You should do it ONLY on errors:
    // request->connection_close = 1;
    return len;
}
void *main_job(void *arg)
{
    int method;
    struct conn_info *conn_info = arg;
    struct request *request = allocate_request();
    request->connection_close = 0;
    pr_info("Starting new session from %s:%d\n",
        inet_ntoa(conn_info->addr.sin_addr),
        ntohs(conn_info->addr.sin_port));
    do {
        method = recv_request(conn_info->socket_fd, request);
        // Insert your operations here
        switch (method) {
        case SET:
            set_request(conn_info->socket_fd, request);
            break;
        case GET:
            break;
        case DEL:
            break;
        case RST:
            // ./check.py issues a reset request after each test
            // to bring back the hashtable to a known state.
            // Implement your reset command here.
            send_response(conn_info->socket_fd, OK, 0, NULL);
            break;
        }
        if (request->key) {
            free(request->key);
        }
    } while (!request->connection_close);
    close_connection(conn_info->socket_fd);
    free(request);
    free(conn_info);
    return (void *)NULL;
}
void ht_init(){
    ht = malloc(sizeof(hashtable_t)); //allocate table
    ht->items = malloc(sizeof(hash_item_t*) * HT_CAPACITY);//allocate entries
    ht->capacity = HT_CAPACITY;
    
    for(int i = 0; i < HT_CAPACITY; i++){//if next is null
        ht->items[i] = NULL;
    }
    
}
int main(int argc, char *argv[])
{
    int listen_sock;
    listen_sock = server_init(argc, argv);
    // Initialuze your hashtable.
    // @see kvstore.h for hashtable struct declaration
    ht_init();
    pthread_t pt;
    for (;;) {
        struct conn_info *conn_info =
            calloc(1, sizeof(struct conn_info));
        if (accept_new_connection(listen_sock, conn_info) < 0) {
            error("Cannot accept new connection");
            free(conn_info);
            continue;
        }
        // main_job(conn_info);
        pthread_create(&pt, NULL,main_job, conn_info);
    }
    return 0;
}