#include <stdio.h>
#include <unistd.h>
#define ALIGNMENT 16   // Must be power of 2
#define PAGE_SIZE 1024
#define GET_PAD(x) ((ALIGNMENT - 1) - ((x) - 1) & (ALIGNMENT - 1))
#define PADDED_SIZE(x) ((x) + GET_PAD(x))
#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))
#define PADDED_BLOCK_SIZE (PADDED_SIZE(sizeof(struct block)))

struct block{
    struct block *next;
    int size;
    int in_use;
};

struct block *head = NULL;

void *myalloc(int size){
    if (head == NULL){ // Initialize heap on first call
        
        printf("Init; Location of inital break point: %p\n", sbrk(0));
        head = (struct block *) sbrk(PAGE_SIZE);
        
        head->next = NULL;
        head->size = PAGE_SIZE - PADDED_BLOCK_SIZE;
        head->in_use = 0;

    }

    struct block *p = head;
    struct block *last;
    do{
        int available_space = p->size; 
        if(p->in_use == 0 && (available_space >= PADDED_SIZE(size))){ // Use first block that is free, and has a enough space to accomodate the padded size, and a new block
            // Found usable space
            // printf("Address of start of block: %p\nAddress of start of data: %p\n", p,  p + 1);

            printf("Allocating %d bytes (%d with padding, %d with padding and padded block) in heap starting at address: %p.\n", size, PADDED_SIZE(size), (PADDED_SIZE(size) + PADDED_BLOCK_SIZE), p);
            p->in_use = 1;
            int remaining_size = available_space - PADDED_SIZE(size);
            
            if(remaining_size >= PADDED_BLOCK_SIZE + ALIGNMENT){ // If there is leftover space in current data block for a new block AND more PADDED data, then we need to create a new node between the current node and its next node
                // printf("Creating intermediary node at end of current nodes data\n");
                p->size = PADDED_SIZE(size);
                struct block *next = p->next;
                p->next = (struct block *) (((char*)(p + 1)) + PADDED_SIZE(size));// Start of next block = Address of start of current block + size of padded block + size of padded data
                p->next->size = remaining_size - PADDED_BLOCK_SIZE; // Size of new node is the size of the old node - size of new data - size of a new node
                p->next->in_use = 0; // New block is not being used
                p->next->next = next; // New Block's next is the old blocks next block
            }else{ 
                printf("Not enough space in current block for intermediary node, Allocating %d bytes to full block starting at %p\n", size, p);
            }
            return p + 1; // Return address to data
        }
        last = p;
        p = p->next;
    }while(p != NULL);
    // If last node is free, grow its size, if its used, make new node
    printf("Could not find space to allocate %d bytes (%d with padding) in heap. Increasing heap size by %d\n", size, PADDED_SIZE(size), PAGE_SIZE);
    if(last->in_use == 1){ 
        // printf("Current end of break point %p\n", sbrk(0));
        
        last->next = (struct block *) sbrk(PAGE_SIZE);
        // printf("New end of break point: %p\n", sbrk(0));
       
        last->next->size = ((char*)(((char*)sbrk(0)) - ((char*) last->next)) - ( (char*) PADDED_BLOCK_SIZE) );
        last->next->in_use = 0;
        print_data();
        
    }else{
        // printf("Current end of break point %p\n", sbrk(0));
        sbrk(PAGE_SIZE); // Set new break point to the old break point + the PAGE_SIZE
        // printf("New end of break point: %p\n", sbrk(0));
        
        last->size = (char*)sbrk(0) - (char*) (last + 1);
        print_data();
    }
    myalloc(size);
}

void myfree(void * p){
    // printf("Freeing block starting at %p\n\n", ((struct block *) p) - 1);
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
    
    int *p = (int*) myalloc(100);
    print_data();
    int *p1 = (int*) myalloc(500);
    print_data();
    myfree(p);
    print_data();
    int *p2 = (int*) myalloc(5000);
    print_data();
    int *p3 = (int*) myalloc(5000);
    print_data();
    myfree(p2);
    print_data();
    myalloc(200);
    print_data();
    
    return 0;
}

