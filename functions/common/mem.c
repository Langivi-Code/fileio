//
// Created by user on 30.11.21.
//
#include <sys/resource.h>
#include <errno.h>
#include "mem.h"
void mem(char * tag) {
    errno = 0;
    struct rusage memory;
    getrusage(RUSAGE_SELF, &memory);
    if(errno == EFAULT)
        printf("Error: EFAULT\n");
    else if(errno == EINVAL)
        printf("Error: EINVAL\n");
    puts(tag);
    printf("Usage: %ld\n", memory.ru_ixrss);
    printf("Usage: %ld\n", memory.ru_isrss);
    printf("Usage: %ld\n", memory.ru_idrss);
    printf("Max: %ld\n", memory.ru_maxrss);
}

