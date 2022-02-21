#include <stdio.h>
#include <unistd.h>
#include "myalloc.h"
#define ALIGNMENT 16   // Must be power of 2
#define PAGE_SIZE 1024
#define GET_PAD(x) ((ALIGNMENT - 1) - ((x) - 1) & (ALIGNMENT - 1))
#define PADDED_SIZE(x) ((x) + GET_PAD(x))
#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))
#define PADDED_BLOCK_SIZE (PADDED_SIZE(sizeof(struct block)))


struct block *head = NULL;

int check_block_availability(struct block *p, int size){
    if(p->in_use == 0 && (p->size >= PADDED_SIZE(size))){ // Use first block that is free, and has a enough space to accomodate the padded size, and a new block
        return 1; // Free block with enough space
    }
    return 0;
}

void split_space(struct block **p, int remaining_size, int size){
    // printf("Creating intermediary node at end of current nodes data\n");
    (*p)->size = PADDED_SIZE(size);
    struct block *next = (*p)->next;
    (*p)->next = (struct block *) (((char*)((*p) + 1)) + PADDED_SIZE(size)); // Start of next block = Address of start of current block + size of padded block + size of padded data
    (*p)->next->size = remaining_size - PADDED_BLOCK_SIZE; // Size of new node is the size of the old node - size of new data - size of a new node
    (*p)->next->in_use = 0; // New block is not being used
    (*p)->next->next = next; // New Block's next is the old blocks next block
}

void *find_space(int size){

    for(struct block *p = head; p != NULL; p = p->next){
        if(check_block_availability(p, size) == 1){ // Find first block with enough available space
            // Usable block found!
            printf("Allocating %d bytes (%d with padding, %ld with padding and padded block) in heap starting at address: %p.\n", size, PADDED_SIZE(size), (PADDED_SIZE(size) + PADDED_BLOCK_SIZE), p);
            int remaining_size = p->size - PADDED_SIZE(size);
            if(remaining_size >= PADDED_BLOCK_SIZE + ALIGNMENT){ // Split space into 2 blocks if there is enough space left over for a block head and aligned data
                split_space(&p, remaining_size, size);
            }else{ 
                printf("Not enough space in current block for intermediary node, Allocating %d bytes to full block starting at %p\n", size, p);
            }
            p->in_use = 1;
            return p; // Return address to block
        }
    }

    return NULL; // Usable block not found, failed to find space
}

void increase_heap_size(int size){
    printf("Could not find space to allocate %d bytes (%d with padding) in heap. Increasing heap size by %d\n", size, PADDED_SIZE(size), PAGE_SIZE);

    struct block *p = head;
    
    while(p->next != NULL){
        p = p->next;
    }

    if(p->in_use == 1){ // Create new block at end of heap
        p->next = (struct block *) sbrk(PAGE_SIZE);
        p->next->size = ((char*)(((char*)sbrk(0)) - ((char*) p->next)) - ( (char*) PADDED_BLOCK_SIZE) );
        p->next->in_use = 0;
        print_data();
        
    }else{ // Grow free block at end of heap
        sbrk(PAGE_SIZE); // Increment the break point by PAGE_SIZE
        p->size = (char*)sbrk(0) - (char*) (p + 1);
        print_data();
    }   
}

void initialize_heap(){ // Create tart of list, and intiialze all its values
    printf("Initial Location of inital break point: %p\n", sbrk(0)); // For some reason this print statment is necessary to keep successive calls to brk from incrimenting the break point by 130K bytes
    head = (struct block *) sbrk(PAGE_SIZE);
    head->next = NULL;
    head->size = PAGE_SIZE - PADDED_BLOCK_SIZE;
    head->in_use = 0;
}

void *myalloc(int size){
    if (head == NULL){ // Initialize heap on first call
        initialize_heap();
    }
    struct block *p = (struct block *) find_space(size); // Returns pointer to available block of memory, or NULL if none available
    for(p; p == NULL; increase_heap_size(size), p = find_space(size)); // Increase the heap size then find space, until find space succeeds (p is not null)

    return p + 1; // Return pointer to data (instead of block)
}

void myfree(void * p){
    printf("Freeing block starting at %p\n", ((struct block *) p) - 1);
    (((struct block *) p) - 1)->in_use = 0; // Set the block in use value to 0. The block is 1 block size behind the start of the data pointer
}

void print_data(void)
{
    struct block *b = head;

    if (b == NULL) {
        printf("[empty]\n");
        return;
    }

    while (b != NULL) {
        // Uncomment the following line if you want to see the pointer values
        printf("[%p:%d,%s]", b, b->size, b->in_use? "used": "free");
        //printf("[%d,%s]", b->size, b->in_use? "used": "free");
        if (b->next != NULL) {
            printf(" -> ");
        }

        b = b->next;
    }

    printf("\n\n");
}

int main(void){

    myalloc(1008);     print_data();
    myalloc(48);       print_data();
    myalloc(50);       print_data();
    myalloc(50);       print_data();
    myalloc(784);      print_data();
    myalloc(32);       print_data();
    
    return 0;
}