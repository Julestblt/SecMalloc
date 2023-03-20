#ifndef _SECMALLOC_PRIVATE_H
#define _SECMALLOC_PRIVATE_H

/*
 * Ici vous pourrez faire toutes les déclarations de variables/fonctions pour votre usage interne
 * */

// Structure qui va contenir les informations sur la mémoire allouée.
typedef struct s_block
{
    size_t size;
    struct s_block *next;
    int free;
    void *ptr;
    char data[1];
    unsigned long canary;
} t_block;

#define BLOCK_SIZE (sizeof(t_block))

#define CANARY_SIZE (sizeof(unsigned long))

#endif
