// cette définition permet d'accéder à mremap lorsqu'on inclue sys/mman.h
#define _GNU_SOURCE
#include <sys/mman.h>
#include "my_secmalloc.h"
#include "my_secmalloc_private.h"

void    *my_malloc(size_t size)
{
    (void) size;
    return NULL;
}

void    my_free(void *ptr)
{
    (void) ptr;
}

void    *my_calloc(size_t nmemb, size_t size)
{
    (void) nmemb;
    (void) size;
    return NULL;
}

void    *my_realloc(void *ptr, size_t size)
{
    (void) ptr;
    (void) size;
    return NULL;

}

#ifdef DYNAMIC
/*
 * Lorsque la bibliothèque sera compilé en .so les symboles malloc/free/calloc/realloc seront visible
 * */

void    *malloc(size_t size)
{
    return my_malloc(size);
}
void    free(void *ptr)
{
    my_free(ptr);
}
void    *calloc(size_t nmemb, size_t size)
{
    return my_calloc(nmemb, size);
}

void    *realloc(void *ptr, size_t size)
{
    return my_realloc(ptr, size);
}

#endif
