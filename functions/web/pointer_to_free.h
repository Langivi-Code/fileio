#include <inttypes.h>

//
// Created by admin on 10.12.2021.
//
#ifndef FILEIO_POINTER_TO_FREE_H
#define FILEIO_POINTER_TO_FREE_H
typedef struct pntrs_to_free{
    u_int8_t size;
    void * * pointers;
} pntrs_to_free;
#endif //FILEIO_POINTER_TO_FREE_H
