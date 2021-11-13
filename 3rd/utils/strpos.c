//
// Created by admin on 13.11.2021.
//
#include "strpos.h"
uintptr_t strpos(char *text, char * delim) {
    char *p;
    p = strstr(text, delim);
    return (p-text);
}