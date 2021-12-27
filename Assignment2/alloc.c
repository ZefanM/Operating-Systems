#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct obj_metadata{
    size_t size;
    struct obj_metadata *next;
    unsigned int is_free;
}metadata_t;

#define STRUCT_SIZE sizeof(struct obj_metadata)                

metadata_t *head = NULL;                                        //head pointer
metadata_t *prev = NULL;                                        //prev ponter of head
int bool = 0;                                                   //boolean value to check if there are free blocks
void *trash = NULL;

size_t align_8(size_t size){                                     //formula to align to 8
    int y;
    y = (size / 8) + (((size % 8 )!= 0));
    return y * 8;
}

void *mymalloc(size_t size)
{
    if (size <= 0) return &trash;

    if (size % 8 != 0)
         size = align_8(size);

    metadata_t *temp = head;
   

    if (bool == 1){
        while (temp != NULL){
            if (temp->is_free == 1 && size <= temp->size){
                temp->is_free = 0;
                temp->size = size;
                return temp + 1;
            }
            temp = temp->next;
        }
    }

    void *new = sbrk(size + STRUCT_SIZE);

    if (new == (void *) - 1) return NULL;

    temp = new;
    temp->size = size;
    temp->next = NULL;

    if (head)
        prev->next = temp;
    else
        head = temp; 

    prev = temp;
    return temp + 1;
}

void *mycalloc(size_t nmemb, size_t size)
{
    if (nmemb == 0 || size == 0) return NULL;
    void *temp = mymalloc(nmemb * size);
    if (temp == NULL) return NULL;
    memset(temp,0, align_8(nmemb * size));
    return temp;
}

void myfree(void *ptr)
{
    if (ptr == NULL) return;
    struct obj_metadata *tmp = ptr - STRUCT_SIZE;
    tmp->is_free = 1;
    bool = 1;
}

void *myrealloc(void *ptr, size_t size)
{
    if (ptr == NULL) return mymalloc(size);
    struct obj_metadata *tmp = ptr - STRUCT_SIZE;
    if (align_8(size) <= tmp->size) return ptr;
    void *new = mymalloc(size);
    memcpy(new, ptr, tmp->size);
    myfree(ptr);
    return new; 
}

/*
 * Enable the code below to enable system allocator support for your allocator.
 * Doing so will make debugging much harder (e.g., using printf may result in
 * infinite loops).
 */
#if 1
void *malloc(size_t size) { return mymalloc(size); }
void *calloc(size_t nmemb, size_t size) { return mycalloc(nmemb, size); }
void *realloc(void *ptr, size_t size) { return myrealloc(ptr, size); }
void free(void *ptr) { myfree(ptr); }
#endif