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

    int success = 0;
    struct block *p = head;
    do{
        if(p->in_use == 0 && p->size > size){
            // Found usable space
            // printf("Address of start of block: %p\nAddress of start of data: %p\n", p,  p + 1);
            p->in_use = 1;
            success = 1;
        }
    }while(p = p->next != NULL);
    if(success == 0){
        // printf("Could not find space to allocate %d bytes in heap\n", size);
        return NULL;
    }
    return p + 1;
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
        // printf("[%p:%d,%s]", b, b->size, b->in_use? "used": "free");
        printf("[%d,%s]", b->size, b->in_use? "used": "free");
        if (b->next != NULL) {
            printf(" -> ");
        }

        b = b->next;
    }

    printf("\n");
}

int main(void){
    
    print_data();
    int *p = (int * ) myalloc(2000);
    print_data();
    int *p1 = (int * ) myalloc(20);
    print_data();
    
    return 0;
}

