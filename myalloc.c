#include <stdio.h>
#include <unistd.h>
#define ALIGNMENT 16   // Must be power of 2
#define GET_PAD(x) ((ALIGNMENT - 1) - ((x) - 1) & (ALIGNMENT - 1))
#define PADDED_SIZE(x) ((x) + GET_PAD(x))
#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))

struct block{
    struct block *next;
    int size;
    int in_use;
};

struct block *head = NULL;

void *myalloc(int size){
    if (head == NULL){
        void *heap = sbrk(1024);
        head = heap;
        head->next = NULL;
        head->size = 1024 - PADDED_SIZE(sizeof(struct block));
        head->in_use = 0;
    }

    struct block *p = head;
    do{
        int available_space = p->next == NULL? p->size - PADDED_SIZE(sizeof(struct block)) : p->size; // If next is null, we need extra space for the data and a new block for the remaining space. A node's size is already padded, so no need to pad again
        if(p->in_use == 0 && (available_space >= PADDED_SIZE(size))){ // Use first block that is free, and has a enough space to accomodate the padded size, and a new block
            // Found usable space
            // printf("Address of start of block: %p\nAddress of start of data: %p\n", p,  p + 1);

            printf("Allocating %d bytes (%d with padding, %d with padding and padded block) in heap at address: %p.\n", size, PADDED_SIZE(size), (PADDED_SIZE(size) + PADDED_SIZE(sizeof(struct block))), p);
            p->in_use = 1;
            
            if(p->next == NULL){ // If there is no next block, we need to create one
                p->size = PADDED_SIZE(size);
                p->next = (struct block *) (((char*)(p + 1)) + PADDED_SIZE(size));// Start of next block = Address of start of current block + size of padded block + size of padded data
                p->next->size = ((char*)head + 1024) - ((char*)(p->next) + PADDED_SIZE(sizeof(struct block))); // Size is the remaining bytes in the allocated heap (Address of last value in heap - (address of last node + size of head of node))
                p->next->in_use = 0; // Next is not being used
                p->next->next = NULL; // Next's next is NULL
            }else{ // IF its not null, we should move the subsequent nodes as far back as possible to make more space
                // struct block * curr_block = p->next;
                // while(curr_block != NULL){
                //     struct block *moved_block = (struct block *) (((char*)(curr_block + 1)) + PADDED_SIZE(size));
                    
                //     moved_block->next = curr_block;
                //     moved_block->size = curr_block->size;
                //     moved_block->in_use = curr_block->in_use;
                //     curr_block->next = moved_block;
                //     curr_block = curr_block->next;
                // }
                // // Once p.next == NULL
                // struct block *moved_block = (struct block *) (((char*)(curr_block + 1)) + PADDED_SIZE(size));
                // moved_block->in_use = curr_block->in_use;
                // moved_block->next = NULL;
                // moved_block->size = ((char*)head + 1024) - (curr_block->size);
                // curr_block = moved_block;
            }
            return p + 1; // Return address to data
        }
        p = p->next;
    }while(p != NULL);
    printf("Could not find space to allocate %d bytes (%d with padding) in heap. Returning NULL\n", size, PADDED_SIZE(size));
    return NULL;
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
    
    int *p = (int * ) myalloc(700);
    print_data();
    
    int *p1 = (int* ) myalloc(30);
    print_data();
    int *p2 = (int*) myalloc(10);
    print_data();
    myfree(p);
    print_data();
    
    int *p3 = (int * ) myalloc(10);
    print_data();
    myfree(p2);
    print_data();
    myalloc(10);
    print_data();
    return 0;
}

