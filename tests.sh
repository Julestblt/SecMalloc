#!/bin/bash
gcc -o test_my_secmalloc test/test.c src/my_secmalloc.c -lcriterion -Iinclude
./test_my_secmalloc
