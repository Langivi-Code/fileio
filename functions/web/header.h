//
// Created by user on 22.10.21.
//

#ifndef FILEIO_HEADER_H
#define FILEIO_HEADER_H

#include <zend_types.h>

#ifndef PROP
#define PROP(string)  string, sizeof(string) - 1
#endif

#include <zend_types.h>
#include "../http/request.h"

zend_string *stringify(key_value_collection collection);

void parse(char *headers, size_t len, zend_object *request);
bool fill_super_global(const unsigned char name, zval * value);
#endif //FILEIO_HEADER_H
