//
// Created by user on 22.10.21.
//

#ifndef FILEIO_HEADER_H
#define FILEIO_HEADER_H

#include <zend_types.h>

void parse(char * headers, size_t len, zend_object * request);
#endif //FILEIO_HEADER_H
