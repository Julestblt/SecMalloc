// cette définition permet d'accéder à mremap lorsqu'on inclue sys/mman.h
#define _GNU_SOURCE
#include <sys/mman.h>
#include "my_secmalloc.h"
#include "my_secmalloc_private.h"

t_block *g_base = NULL;

// Fonction qui permet de générer un canary aléatoire.
unsigned long generate_canary()
{
    return (rand() % 1000000);
}

// Fonction qui permet de trouver un bloc libre de la taille demandée.
t_block *find_free_block(t_block **last, size_t size)
{
    // On parcours la liste chainée jusqu'à trouver un bloc libre de la taille demandée.
    t_block *current = g_base;
    // On stocke le dernier bloc pour pouvoir le lier au nouveau bloc.
    while (current && !(current->free && current->size >= (size + sizeof(unsigned long))))
    {
        // On stocke le dernier bloc pour pouvoir le lier au nouveau bloc.
        *last = current;
        // On passe au bloc suivant.
        current = current->next;
    }
    // On retourne le bloc libre trouvé ou NULL si aucun bloc libre n'a été trouvé.
    return (current);
}

// Fonction qui permet d'allouer de la mémoire.
t_block *request_space(t_block *last, size_t size)
{
    // On alloue de la mémoire pour le nouveau bloc.
    t_block *new_block;
    new_block = mmap(0, size + BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // On vérifie que l'allocation s'est bien passée.
    if (new_block == MAP_FAILED)
    {
        return (NULL);
    }

    // On initialise le nouveau bloc.
    new_block->size = size;
    new_block->next = NULL;
    new_block->free = 0;
    new_block->ptr = new_block->data;
    new_block->canary = generate_canary();

    // On lie le nouveau bloc au dernier bloc.
    if (last)
    {
        last->next = new_block;
    }

    // On retourne le nouveau bloc.
    return (new_block);
}

void *my_malloc(size_t size)
{
    // On vérifie que la taille demandée est valide.
    if (size <= 0)
    {
        return (NULL);
    }

    // On vérifie si la liste chainée est vide.
    if (!g_base)
    {
        // On alloue de la mémoire pour le nouveau bloc.
        t_block *new_block = request_space(NULL, size);

        // On vérifie que l'allocation s'est bien passée, sinon on retourne NULL.
        if (!new_block)
        {
            return (NULL);
        }

        // On initialise la liste chainée.
        g_base = new_block;
    }
    // Si la liste chainée n'est pas vide.
    else
    {
        // On cherche un bloc libre de la taille demandée.
        t_block *last_block = g_base;

        // On stocke le dernier bloc pour pouvoir le lier au nouveau bloc.
        t_block *free_block = find_free_block(&last_block, size);

        // On vérifie si un bloc libre a été trouvé.
        if (free_block)
        {
            // Si oui, on defini le bloc comme occupé.
            free_block->free = 0;
        }
        // Si aucun bloc libre n'a été trouvé.
        else
        {
            // On alloue de la mémoire pour le nouveau bloc.
            free_block = request_space(last_block, size);

            // On vérifie que l'allocation s'est bien passée, sinon on retourne NULL.
            if (!free_block)
            {
                return (NULL);
            }

            // On retourne l'adresse de la mémoire allouée.
            return (free_block->ptr);
        }
    }

    // On retourne l'adresse de la mémoire allouée.
    return (g_base->ptr);
}

void my_free(void *ptr)
{
    (void)ptr;
}

void *my_calloc(size_t nmemb, size_t size)
{
    // On vérifie que la taille demandée est valide.
    if (nmemb == 0 || size == 0)
    {
        return (NULL);
    }

    // On calcule la taille totale de la mémoire à allouer.
    size_t total_size = nmemb * size;

    // On alloue de la mémoire.
    void *ptr = my_malloc(total_size);

    // On vérifie que l'allocation s'est bien passée.
    if (ptr == NULL)
    {
        return (NULL);
    }

    // On initialise la mémoire à 0.
    for (size_t i = 0; i < total_size; i++)
    {
        ((char *)ptr)[i] = 0;
    }

    // On retourne l'adresse de la mémoire allouée.
    return (ptr);
}

void *my_realloc(void *ptr, size_t size)
{
    (void)ptr;
    (void)size;
    return NULL;
}

#ifdef DYNAMIC
/*
 * Lorsque la bibliothèque sera compilé en .so les symboles malloc/free/calloc/realloc seront visible
 * */

void *malloc(size_t size)
{
    return my_malloc(size);
}
void free(void *ptr)
{
    my_free(ptr);
}
void *calloc(size_t nmemb, size_t size)
{
    return my_calloc(nmemb, size);
}

void *realloc(void *ptr, size_t size)
{
    return my_realloc(ptr, size);
}

#endif
