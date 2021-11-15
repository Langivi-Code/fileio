//
// Created by user on 22.10.21.
//

#ifndef FILEIO_HEADER_H
#define FILEIO_HEADER_H

#include <zend_types.h>

#ifndef PROP
#define PROP(string)  string, sizeof(string) - 1
#endif
struct key_value {
    char key[UINT16_MAX];
    char value[UINT16_MAX];
};
struct uri_parsed {
    char uri[UINT16_MAX];
    uint8_t qs_size;
    struct key_value *get_qs;
};

struct input_data {
    zval headers;
    char * qs;
    char * cur_header;
};
void parse(char *headers, size_t len, zend_object *request);

#endif //FILEIO_HEADER_H
