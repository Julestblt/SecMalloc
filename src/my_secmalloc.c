// cette définition permet d'accéder à mremap lorsqu'on inclue sys/mman.h
#define _GNU_SOURCE
#include <sys/mman.h>
#include "my_secmalloc.h"
#include "my_secmalloc_private.h"

void *my_malloc(size_t size)
{
    // Si size est 0, malloc retourne NULL.
    if (size == 0)
    {
        return (NULL);
    }

    /**
     * On alloue la mémoire avec mmap.
     * Le premier paramètre prends l'adresse de la mémoire à allouer, si on met NULL, mmap choisit l'adresse.
     * Pour l'implémentation du canari il faudra surement mettre une adresse fixe.
     * */
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // Si mmap retourne MAP_FAILED, il y a eu une erreur donc on retourne NULL.
    if (ptr == MAP_FAILED)
    {
        return (NULL);
    }

    // On retourne l'adresse de la mémoire allouée.
    return ptr;
}

void my_free(void *ptr)
{
    // Si ptr est NULL, free ne fait rien.
    if (!ptr)
    {
        return;
    }

    // On libère la mémoire avec munmap.
    munmap(ptr, 0);

    // On met ptr à NULL pour éviter les problèmes.
    ptr = NULL;

    return;
}

void *my_calloc(size_t nmemb, size_t size)
{
    // TODO: Vérifier si size * nmemb dépasse la taille d'un size_t.
    (void)nmemb;

    // Si size est 0, malloc retourne NULL.
    if (size == 0)
    {
        return (NULL);
    }

    // On alloue la mémoire avec mmap.
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // Si mmap retourne MAP_FAILED, il y a eu une erreur donc on retourne NULL.
    if (ptr == MAP_FAILED)
    {
        return (NULL);
    }

    // On met à 0 la mémoire allouée.
    for (size_t i = 0; i < size; i++)
    {
        ((char *)ptr)[i] = 0;
    }

    // On retourne l'adresse de la mémoire allouée.
    return ptr;
}

void *my_realloc(void *ptr, size_t size)
{
    // Si size est 0, malloc retourne NULL.
    if (size == 0)
    {
        return (NULL);
    }

    // Si ptr est NULL, realloc est équivalent à malloc.
    if (!ptr)
    {
        return (my_malloc(size));
    }

    // On alloue la mémoire avec mmap.
    void *new_ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // Si mmap retourne MAP_FAILED, il y a eu une erreur donc on retourne NULL.
    if (new_ptr == MAP_FAILED)
    {
        return (NULL);
    }

    // On copie la mémoire de ptr dans new_ptr.
    for (size_t i = 0; i < size; i++)
    {
        ((char *)new_ptr)[i] = ((char *)ptr)[i];
    }

    // On libère la mémoire avec munmap.
    munmap(ptr, 0);

    // On met ptr à NULL pour éviter les problèmes.
    ptr = NULL;

    // On retourne l'adresse de la mémoire allouée.
    return new_ptr;
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
