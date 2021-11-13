//
// Created by user on 22.10.21.
//

#ifndef FILEIO_HEADER_H
#define FILEIO_HEADER_H

#include <zend_types.h>
#ifndef PROP
    #define PROP(string)  string, sizeof(string) - 1
#endif
void parse(char * headers, size_t len, zend_object * request);
#endif //FILEIO_HEADER_H
