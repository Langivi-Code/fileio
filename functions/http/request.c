//
// Created by admin on 15.11.2021.
//

#include "request.h"
#include "../../3rd/llhttp/llhttp.h"
#include <php.h>

#include "../../3rd/utils/strpos.h"
#include "ext/standard/php_var.h"
#include "../common/mem.h"


//TODO REWRITE TO OWN IMPL
static inline int url_htoi(char *s) {
    int value;
    int c;

    c = ((unsigned char *) s)[0];
    if (isupper(c)) {
        c = tolower(c);
    }
    value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

    c = ((unsigned char *) s)[1];
    if (isupper(c)) {
        c = tolower(c);
    }
    value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

    return (value);
}
//TODO REWRITE TO OWN IMPL
/* return value: length of decoded string */
size_t url_decode(char *str, size_t len) {
    char *dest = str;
    char *data = str;

    while (len--) {
        if (*data == '+') {
            *dest = ' ';
        } else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) && isxdigit((int) *(data + 2))) {
            *dest = (char) url_htoi(data + 1);
            data += 2;
            len -= 2;
        } else {
            *dest = *data;
        }
        data++;
        dest++;
    }
    *dest = '\0';

    return dest - str;
}

//TODO REVIEW LIMITS
//TOO implement array parsing
static struct key_value parse_key_value(char *key_value) {
    uintptr_t eq_pos = strpos(key_value, "=");
    struct key_value kv = {};
    uintptr_t val_len = strlen(key_value) - eq_pos;
    memset(kv.key, 0, eq_pos + 1);
    strncpy(kv.key, key_value, eq_pos);
    memset(kv.value, 0, val_len + 1);
    strncpy(kv.value, (key_value + eq_pos + 1), val_len);
    return kv;
}

struct key_value create_kv_pair(const char *key, const char *value) {
    struct key_value kv = {};
    size_t key_len = strlen(key);
    size_t val_len = strlen(value);
    memset(kv.key, 0, key_len + 1);
    strncpy(kv.key, key, key_len);
    memset(kv.value, 0, val_len + 1);
    strncpy(kv.value, value, val_len);
    return kv;

}

key_value_collection create_kv_collection() {
    key_value_collection collection = {.size=0, .kv=NULL};
    return collection;
}

//private don't use separate
static void alloc_kv_collection(key_value_collection *collection) {
    if (collection->kv == NULL) {
        collection->kv = emalloc(sizeof(struct key_value) * (collection->size + 1));
    } else {
        collection->kv = erealloc(collection->kv, sizeof(struct key_value) * (collection->size + 1));
    }
}


key_value_collection *
append_string_to_kv_collection(key_value_collection *  collection, const char *key, const char *value) {
    alloc_kv_collection(collection);
    collection->kv[collection->size] = create_kv_pair(key, value);
    collection->size++;
    return collection;
}

key_value_collection * append_kv_to_collection(key_value_collection * collection, struct key_value kv) {
    alloc_kv_collection(collection);
    collection->kv[collection->size] = kv;
    collection->size++;
    return collection;
}

struct uri_parsed *parse_urlstring(char *urlstring_arg) {
    mem("before alloc headers");
    struct uri_parsed *request_parsed = emalloc(sizeof(struct uri_parsed));
    memset(request_parsed, 0, sizeof(struct uri_parsed));
    uintptr_t qsStart = strpos(urlstring_arg, "?");
//    printf("1st qsStart %lu ----- %s\n", qsStart, querystring_arg);
    if (qsStart == FAILURE) {
        qsStart = strlen(urlstring_arg);
    }
    mem("after alloc headers");
//    printf("2nd qsStart %lu ----- %s\n", qsStart, querystring_arg);
    if ((qsStart + 1) < strlen(urlstring_arg)) {
        uint8_t qs_size = strlen(urlstring_arg) - (qsStart + 1);
//        printf("qs wsize %u\n", qs_size);
        request_parsed->get_qs = create_kv_collection();
        char qs[qs_size + 1];
        memset(qs, 0, qs_size + 1);
        strncpy(qs, (urlstring_arg + qsStart + 1), qs_size);
        char *p;
        p = strtok(qs, "&");
//        puts(p);
        append_kv_to_collection(&request_parsed->get_qs, parse_key_value(p));

//        printf("%s\n", p);
        do {
            p = strtok('\0', "&");
            if (p) {
                append_kv_to_collection(&request_parsed->get_qs, parse_key_value(p));
                printf("%s\n", p);
            }
        } while (p);
        //parse QS
    }
    memset(request_parsed->uri, 0, qsStart + 1);
    strncpy(request_parsed->uri, urlstring_arg, qsStart);
    mem("after save headers");
    return request_parsed;

}

key_value_collection parse_querystring(char *querystring_arg) {
    key_value_collection params = create_kv_collection();
    char *p;
    p = strtok(querystring_arg, "&");
    append_kv_to_collection(&params, parse_key_value(p));
    do {
        p = strtok('\0', "&");
        if (p) {
            append_kv_to_collection(&params, parse_key_value(p));
            printf("%s\n", p);
        }
    } while (p);
    return params;
}