#include <criterion/criterion.h>
#include <string.h>
#include "my_secmalloc.h"

Test(my_secmalloc, malloc_and_free)
{
    // Test d'allocation et libération simple.
    void *ptr = my_malloc(100);
    cr_assert(ptr != NULL, "my_malloc a retourné NULL.");
    my_free(ptr);
}

Test(my_secmalloc, calloc_and_free)
{
    // Test d'allocation et libération avec calloc.
    void *ptr = my_calloc(10, 10);
    cr_assert(ptr != NULL, "my_calloc a retourné NULL.");

    // Vérification de l'initialisation à 0.
    for (size_t i = 0; i < 100; i++)
    {
        cr_assert(((char *)ptr)[i] == 0, "my_calloc n'a pas initialisé la mémoire à 0.");
    }
    my_free(ptr);
}

Test(my_secmalloc, realloc_and_free)
{
    // Test d'allocation, réallocation et libération.
    void *ptr = my_malloc(50);
    cr_assert(ptr != NULL, "my_malloc a retourné NULL.");

    void *new_ptr = my_realloc(ptr, 100);
    cr_assert(new_ptr != NULL, "my_realloc a retourné NULL.");

    my_free(new_ptr);
}

Test(my_secmalloc, invalid_ptr)
{
    // Test de libération avec un pointeur invalide.
    my_free(NULL); // Cette opération ne doit pas causer de plantage.
}

Test(my_secmalloc, multiple_allocations)
{
    // Test d'allocation et libération de plusieurs blocs.
    void *ptr1 = my_malloc(100);
    void *ptr2 = my_malloc(200);
    void *ptr3 = my_malloc(300);

    cr_assert(ptr1 != NULL, "my_malloc a retourné NULL pour ptr1.");
    cr_assert(ptr2 != NULL, "my_malloc a retourné NULL pour ptr2.");
    cr_assert(ptr3 != NULL, "my_malloc a retourné NULL pour ptr3.");

    my_free(ptr1);
    my_free(ptr2);
    my_free(ptr3);
}

Test(my_secmalloc, multiple_allocations_and_realloc)
{
    // Test d'allocation, réallocation et libération de plusieurs blocs.
    void *ptr1 = my_malloc(100);
    void *ptr2 = my_malloc(200);
    void *ptr3 = my_malloc(300);

    cr_assert(ptr1 != NULL, "my_malloc a retourné NULL pour ptr1.");
    cr_assert(ptr2 != NULL, "my_malloc a retourné NULL pour ptr2.");
    cr_assert(ptr3 != NULL, "my_malloc a retourné NULL pour ptr3.");

    void *new_ptr1 = my_realloc(ptr1, 200);
    void *new_ptr2 = my_realloc(ptr2, 300);
    void *new_ptr3 = my_realloc(ptr3, 400);

    cr_assert(new_ptr1 != NULL, "my_realloc a retourné NULL pour new_ptr1.");
    cr_assert(new_ptr2 != NULL, "my_realloc a retourné NULL pour new_ptr2.");
    cr_assert(new_ptr3 != NULL, "my_realloc a retourné NULL pour new_ptr3.");

    my_free(new_ptr1);
    my_free(new_ptr2);
    my_free(new_ptr3);
}

Test(my_secmalloc, zero_size_allocations)
{
    // Test d'allocation avec une taille de 0.
    void *ptr = my_malloc(0);
    cr_assert(ptr == NULL, "my_malloc aurait dû retourner NULL pour une taille de 0.");
}

Test(my_secmalloc, negative_size_allocations)
{
    // Test d'allocation avec une taille négative.
    void *ptr = my_malloc(-1);
    cr_assert(ptr == NULL, "my_malloc aurait dû retourner NULL pour une taille négative.");
}

Test(my_secmalloc, realloc_smaller)
{
    // Test de réallocation avec une taille plus petite.
    void *ptr = my_malloc(100);
    cr_assert(ptr != NULL, "my_malloc a retourné NULL.");

    void *new_ptr = my_realloc(ptr, 50);
    cr_assert(new_ptr != NULL, "my_realloc a retourné NULL pour une taille plus petite.");

    my_free(new_ptr);
}

Test(my_secmalloc, realloc_null_ptr)
{
    // Test de réallocation avec un pointeur NULL.
    void *new_ptr = my_realloc(NULL, 100);
    cr_assert(new_ptr != NULL, "my_realloc aurait dû se comporter comme my_malloc avec un pointeur NULL.");

    my_free(new_ptr);
}

Test(my_secmalloc, realloc_zero_size)
{
    // Test de réallocation avec une taille de 0.
    void *ptr = my_malloc(100);
    cr_assert(ptr != NULL, "my_malloc a retourné NULL.");

    void *new_ptr = my_realloc(ptr, 0);
    cr_assert(new_ptr == NULL, "my_realloc aurait dû se comporter comme my_free avec une taille de 0.");

    // Comme my_realloc avec une taille de 0 est équivalent à my_free, on ne doit pas libérer à nouveau le pointeur.
}

Test(my_secmalloc, data_integrity)
{
    // Test de l'intégrité des données lors de l'allocation et la réallocation.
    const char *data = "Voici des données pour tester l'intégrité.";
    size_t data_len = strlen(data) + 1; // +1 pour le caractère de fin de chaîne.

    char *ptr = my_malloc(data_len);
    cr_assert(ptr != NULL, "my_malloc a retourné NULL.");

    strcpy(ptr, data);

    size_t new_data_len = data_len * 2;
    char *new_ptr = my_realloc(ptr, new_data_len);
    cr_assert(new_ptr != NULL, "my_realloc a retourné NULL.");

    cr_assert(strcmp(new_ptr, data) == 0, "Les données ne sont pas identiques après la réallocation.");

    my_free(new_ptr);
}
