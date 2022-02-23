#define ALIGNMENT 16   // Must be power of 2
#define PAGE_SIZE 1024
#define GET_PAD(x) (((ALIGNMENT - 1) - ((x) - 1)) & (ALIGNMENT - 1))
#define PADDED_SIZE(x) ((x) + GET_PAD(x))
#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))
#define PADDED_BLOCK_SIZE (PADDED_SIZE(sizeof(struct block)))

struct block{
    struct block *next;
    int size;
    int in_use;
};

void print_data(void);
void myfree(void * p);
void coalesce(void);