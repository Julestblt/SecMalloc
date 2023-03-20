// cette définition permet d'accéder à mremap lorsqu'on inclue sys/mman.h
#define _GNU_SOURCE
#include <sys/mman.h>
#include "my_secmalloc.h"
#include "my_secmalloc_private.h"

void *my_malloc(size_t size)
{
    // On vérifie que la taille demandée n'est pas 0, si c'est le cas on retourne NULL.
    if (size == 0)
    {
        return (NULL);
    }

    // On utilise mmap pour allouer de la mémoire.
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // On vérifie que mmap n'a pas retourné d'erreur, si c'est le cas on retourne NULL.
    if (ptr == MAP_FAILED)
    {
        return (NULL);
    }

    // On retourne l'adresse de la mémoire allouée.
    return (ptr);
}

void my_free(void *ptr)
{
    (void)ptr;
}

void *my_calloc(size_t nmemb, size_t size)
{
    (void)nmemb;
    (void)size;
    return NULL;
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
