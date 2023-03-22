#include <criterion/criterion.h>
#define _GNU_SOURCE
#include <sys/mman.h>
#include <stdio.h>
#include "my_secmalloc.h"
#include "my_secmalloc_private.h"

// Test simple de malloc.
Test(my_malloc, simple_test)
{
    char *str = malloc(15);
    cr_expect_not_null(str);
}
