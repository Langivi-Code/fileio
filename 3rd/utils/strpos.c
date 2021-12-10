//
// Created by admin on 13.11.2021.
//
#include <stdint.h>
#include "strpos.h"
#include "stdio.h"
intptr_t strpos(char *text, char *delim) {
    char *p;
    p = strstr(text, delim);
//    printf(" is null - %u %p %p %s %s\n", p == NULL, p, NULL, text, delim);
    return p != NULL ? (p - text) : -1;
}