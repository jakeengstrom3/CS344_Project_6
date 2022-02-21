struct block{
    struct block *next;
    int size;
    int in_use;
};

void print_data(void);
void myfree(void * p);
