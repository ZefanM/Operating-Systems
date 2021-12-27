#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
struct obj_metadata
{
    size_t size;
    struct obj_metadata *next;
    int is_free;
};
struct obj_metadata *temp_block = NULL;
struct obj_metadata *current = NULL;
struct obj_metadata *previous = NULL;
struct obj_metadata *split(struct obj_metadata *the_free_block, size_t size);
void merge(struct obj_metadata *freed_block);
void set_values(struct obj_metadata *n, size_t size)
{
    n->size = size;
    n->is_free = 0;
    n->next = NULL;
}
int number_of_free = 0;
size_t align(size_t size)
{
    return (((size) + (sizeof(long) - 1)) & ~(sizeof(long) - 1));
}
void *mymalloc(size_t size_user)
{
    if (size_user <= 0)
    {
        return NULL;
    }
    size_t size = align(size_user + align(sizeof(size_t)));

    if (number_of_free > 0)
    {
        struct obj_metadata *temp = current;
        while (temp != NULL)
        {
            if (temp->is_free == 1 && size <= temp->size)
            {

                if (((size) + sizeof(struct obj_metadata)) < temp->size)
                {
                    temp = split(temp, size);
                }
                temp->is_free = 0;
                return temp + 1;
            }
            temp = temp->next;
        }
    }

    temp_block = sbrk(size + sizeof(struct obj_metadata));
    if (current == NULL)
    {
        set_values(temp_block, size);
        current = temp_block;
    }
    else
    {
        set_values(temp_block, size);
        previous->next = temp_block;
    }
    previous = temp_block;
    return temp_block + 1;
}
struct obj_metadata *split(struct obj_metadata *the_free_block, size_t size)
{
    struct obj_metadata *new_block = (void *)((void *)the_free_block + size + sizeof(struct obj_metadata));
    // ((the_free_block + 1) + size);

    new_block->is_free = 1;
    new_block->size = the_free_block->size - (size - sizeof(struct obj_metadata));
    new_block->next = the_free_block->next;

    the_free_block->next = new_block;
    the_free_block->size = size;
    return the_free_block;
}
void *mycalloc(size_t nmemb, size_t size)
{
    void *temp_ptr = mymalloc(nmemb * size);
    if (!temp_ptr)
        return NULL;
    bzero(temp_ptr, align(nmemb * size));
    return temp_ptr;
}
void myfree(void *ptr)
{
    if (!ptr)
        return;
    temp_block = (struct obj_metadata *)(ptr - sizeof(struct obj_metadata));
    temp_block->is_free = 1;
    if (temp_block->next != NULL && temp_block->next->is_free == 1)
    {
        merge(temp_block);
    }
    number_of_free += 1;
}
void merge(struct obj_metadata *freed_block)
{
    freed_block->size = freed_block->size + freed_block->next->size + sizeof(struct obj_metadata); // add the size of my current block plus the size of next block
    freed_block->next = freed_block->next->next;                                                   // set the next pointer of my current block to the next next pointer
}
void *myrealloc(void *ptr, size_t size)
{
    if (!ptr)
        return NULL;
    temp_block = (struct obj_metadata *)(ptr - sizeof(struct obj_metadata));

    if (temp_block->size >= align(size))
    {
        return ptr;
    }
    void *head = mymalloc(size);
    memcpy(head, ptr, align(size));
    if (ptr != NULL)
    {
        myfree(ptr);
    }
    return head;
}

/*
 * Enable the code below to enable system allocator support for your allocator.
 * Doing so will make debugging much harder (e.g., using printf may result in
 * infinite loops).
 */
#if 1
void *malloc(size_t size)
{
    return mymalloc(size);
}
void *calloc(size_t nmemb, size_t size) { return mycalloc(nmemb, size); }
void *realloc(void *ptr, size_t size) { return myrealloc(ptr, size); }
void free(void *ptr) { myfree(ptr); }
#endif

