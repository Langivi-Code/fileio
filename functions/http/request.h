//
// Created by admin on 15.11.2021.
//

#ifndef FILEIO_REQUEST_HH
#define FILEIO_REQUEST_HH
#include <stdint.h>
#include <stddef.h>
#include <php.h>

#ifndef PROP
#define PROP(string)  string, sizeof(string) - 1
#endif
struct key_value {
    char key[UCHAR_MAX];
    char value[UCHAR_MAX];
};

typedef struct {
    size_t size;
    struct key_value *kv;
} key_value_collection;

struct uri_parsed {
    char uri[UCHAR_MAX];
    key_value_collection get_qs;
};

struct input_data {
    zval headers;
    char *qs;
    char *cur_header;
};

struct uri_parsed *parse_urlstring(char *urlstring_arg);

key_value_collection parse_querystring(char *querystring_arg);

static struct key_value parse_key_value(char *key_value);

struct key_value create_kv_pair(const char *key, const char *value);

size_t url_decode(char *str, size_t len);

key_value_collection create_kv_collection();

key_value_collection *
append_string_to_kv_collection(key_value_collection * collection, const char *key, const char *value);

key_value_collection * append_kv_to_collection(key_value_collection * collection, struct key_value kv) ;

#endif //FILEIO_REQUEST_HH
