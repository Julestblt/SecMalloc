#include <criterion/criterion.h>
#define _GNU_SOURCE
#include <sys/mman.h>
#include <stdio.h>
#include "my_secmalloc.h"
#include "my_secmalloc_private.h"

Test(mmap, simple) {
    // Question: Est-ce que printf fait un malloc ?
    printf("Ici on fait un test simple de mmap\n");
    void *ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    cr_expect(ptr != NULL);
    int res = munmap(ptr, 4096);
    cr_expect(res == 0);
    char *str = my_malloc(15);
    strncpy(str, "ca marche", 15);
    printf("bla %s\n", str);
}
