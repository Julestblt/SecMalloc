// cette définition permet d'accéder à mremap lorsqu'on inclue sys/mman.h
#define _GNU_SOURCE
#include <sys/mman.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "my_secmalloc.h"
#include "my_secmalloc_private.h"

t_block *g_base = NULL;

// Fonction qui ecris les logs dans un fichier ou dans la console.
void my_log(const char *format, ...)
{
    // On initialise la liste des arguments.
    va_list args;
    va_start(args, format);

    // On obtient la taille necessaire pour stocker la chaine formatée.
    int size_needed = vsnprintf(NULL, 0, format, args);
    va_end(args);

    // On alloue de la mémoire sur la pile pour la chaine formatée.
    char *formatted_str = (char *)alloca(size_needed + 1);

    // On écrit la chaine formatée dans la mémoire allouée.
    va_start(args, format);
    vsnprintf(formatted_str, size_needed + 1, format, args);
    va_end(args);

    // On écrit la chaine formatée dans le fichier ou dans la console.
    char *msm_output = getenv("MSM_OUTPUT");
    if (msm_output)
    {
        // Si MSM_OUTPUT est définie, on écris les logs dans le fichier spécifié
        int fd = open(msm_output, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd != -1)
        {
            // On écrit la chaine formatée dans le fichier.
            write(fd, formatted_str, size_needed);
            write(fd, "\n", 1);
            close(fd);
        }
        else
        {
            perror("Erreur lors de l'ouverture du fichier de log");
        }
    }
    else
    {
        // Si MSM_OUTPUT n'est pas définie, écrivez les logs dans stdout
        write(STDOUT_FILENO, formatted_str, size_needed);
        write(STDOUT_FILENO, "\n", 1);
    }
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
        my_log("Erreur lors de l'allocation de mémoire (request_space)");
        return (NULL);
    }

    // On initialise le nouveau bloc.
    new_block->size = size;
    new_block->next = NULL;
    new_block->free = 0;
    new_block->ptr = new_block->data;
    new_block->canary = CANARY_VALUE;

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
        my_log("Erreur lors de l'allocation de mémoire (my_malloc) : taille invalide");
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
            my_log("Erreur lors de l'allocation de mémoire (my_malloc)");
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
                my_log("Erreur lors de l'allocation de mémoire (my_malloc)");
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
    // On vérifie que le pointeur est valide.
    if (!ptr)
    {
        my_log("Erreur lors de la libération de mémoire (my_free) : pointeur invalide");
        return;
    }

    t_block *current = g_base;

    t_block *previous = NULL;

    // On parcours la liste chainée jusqu'à trouver le bloc correspondant au pointeur.
    while (current && current->ptr != ptr)
    {
        previous = current;
        current = current->next;
    }

    // On vérifie que le bloc a été trouvé.
    if (current)
    {
        // Vérification du canari.
        if (current->canary != CANARY_VALUE)
        {
            my_log("Erreur lors de la libération de mémoire (my_free) : canari invalide");
            return;
        }

        // On libère le bloc.
        current->free = 1;

        // On met à jour la liste chaînée en supprimant le bloc libéré.
        if (previous)
        {
            previous->next = current->next;
        }
        else
        {
            g_base = current->next;
        }

        // On libère la mémoire du bloc.
        munmap(current, current->size + BLOCK_SIZE);

        return;
    }

    return;
}

void *my_calloc(size_t nmemb, size_t size)
{
    // On vérifie que la taille demandée est valide.
    if (nmemb == 0 || size == 0)
    {
        my_log("Erreur lors de l'allocation de mémoire (my_calloc) : taille invalide");
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
    // On vérifie que la taille demandée est valide.
    if (size <= 0)
    {
        my_log("Erreur lors de la réallocation de mémoire (my_realloc) : taille invalide");
        return (NULL);
    }

    // On alloue la mémoire avec my_malloc.
    void *new_ptr = my_malloc(size);

    // On vérifie que l'allocation s'est bien passée, sinon on retourne NULL.
    if (!new_ptr)
    {
        my_log("Erreur lors de la réallocation de mémoire (my_realloc)");
        return (NULL);
    }

    // On copie les données de l'ancienne mémoire vers la nouvelle.
    for (size_t i = 0; i < size; i++)
    {
        ((char *)new_ptr)[i] = ((char *)ptr)[i];
    }

    // On libère l'ancienne mémoire.
    if (ptr)
    {
        my_free(ptr);
    }

    return (new_ptr);
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
