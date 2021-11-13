//
// Created by user on 22.10.21.
//

#include "../../3rd/llhttp/llhttp.h"
#include "header.h"
#include "SAPI.h"
#include "../../fileio_arginfo.h"
#include <php.h>
#include <zend_API.h>
#include "../../3rd/utils/strpos.h"
#include "ext/standard/php_var.h"

PHP_FUNCTION (send_header) {
    zend_string * key, *value;
    char *line;
    ZEND_PARSE_PARAMETERS_START(2, 3)
            Z_PARAM_STR(key)
            Z_PARAM_STR(value)
            Z_PARAM_OPTIONAL

    ZEND_PARSE_PARAMETERS_END();
    bool rep = 1;
    sapi_header_line ctr = {0};
    uint32_t length = ZSTR_LEN(key) + ZSTR_LEN(value) + 2;
    line = emalloc(length);
    memset(line, 0, length);
    strncpy(line, ZSTR_VAL(key), ZSTR_LEN(key));
    strcat(line, ":");
    ctr.line = strcat(line, ZSTR_VAL(value));

    ctr.line_len = length;
    sapi_header_op(rep ? SAPI_HEADER_REPLACE : SAPI_HEADER_ADD, &ctr);
}


static int on_info(llhttp_t *p) {
    printf("some info COMPLITED\n\n");
    return 0;
}


static int on_cinfo(llhttp_t *p) {
    printf("PARSE COMPLITED");
    return 0;
}

char *header_name = NULL;
static char *querystring = NULL;

static int on_status(llhttp_t *p, const char *at, size_t length) {
    printf("STATUS IS is %s\n", llhttp_method_name(p->method));
    printf("at %s %d\n", at, length);
    return 0;
}

static int on_url(llhttp_t *p, const char *at, size_t length) {
    if (querystring == NULL) {
        querystring = emalloc(sizeof(char) * (length + 1));
    }

    memset(querystring, 0, sizeof(char) * (length + 1));
    strncpy(querystring, at, length);
    printf("QS AFTER copy is %zu  %s\n", length, querystring);
    return 0;
}


static int on_header_field(llhttp_t *p, const char *at, size_t length) {
    if (header_name == NULL) {
        header_name = emalloc(sizeof(char) * (length + 1));
    } else {
        header_name = erealloc(header_name, sizeof(char) * (length + 1));
    }
    memset(header_name, 0, sizeof(char) * (length + 1));
    strncpy(header_name, at, length);
    add_assoc_string((zval *) p->data, header_name, "");
    return 0;
}

static int on_header_value(llhttp_t *p, const char *at, size_t length) {
    char *value = emalloc(sizeof(char) * (length + 1));
    memset(value, 0, sizeof(char) * (length + 1));
    strncpy(value, at, length);
    add_assoc_string((zval *) p->data, header_name, value);
    efree(value);
    return 0;
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

static struct uri_parsed *parse_querystring(char *querystring_arg) {
    struct uri_parsed *request_parsed = emalloc(sizeof(struct uri_parsed));
    uintptr_t qsStart = strpos(querystring_arg, "?");
    printf("1st qsStart %lu ----- %s\n", qsStart, querystring_arg);
    if (qsStart == FAILURE) {
        qsStart = strlen(querystring_arg);
    }
    printf("2nd qsStart %lu ----- %s\n", qsStart, querystring_arg);
    if ((qsStart + 1) < strlen(querystring_arg)) {
        uint8_t qs_size = strlen(querystring_arg) - (qsStart + 1);
        printf("qs wsize %u\n", qs_size);
        request_parsed->qs_size = 0;
        char qs[qs_size + 1];
        memset(qs, 0, qs_size + 1);
        strncpy(qs, (querystring_arg + qsStart + 1), qs_size);
        char *p;
        p = strtok(qs, "&");
        ++request_parsed->qs_size;
        request_parsed->get_qs = emalloc(sizeof(struct key_value));
        request_parsed->get_qs[0] = parse_key_value(p);
        printf("%s\n", p);
        do {
            p = strtok('\0', "&");
            if (p) {
                request_parsed->get_qs = erealloc(request_parsed->get_qs,
                                                  sizeof(struct key_value) * (request_parsed->qs_size + 1));
                request_parsed->get_qs[request_parsed->qs_size] = parse_key_value(p);
                ++request_parsed->qs_size;
                printf("%s\n", p);
            }
        } while (p);
        //parse QS
    }
    memset(request_parsed->uri, 0, qsStart + 1);
    strncpy(request_parsed->uri, querystring_arg, qsStart);
    return request_parsed;

}

void parse(char *headers, size_t len, zend_object *request) {
    llhttp_t parser;
//    llhttp_settings_init(&settings);
    zval zv_headers;
    zval zv_qs;
    array_init(&zv_headers); //  zend_read_property(request->ce, request, PROP("headers"), 0, NULL);


    llhttp_settings_t settings = {
            .on_message_begin = NULL,
            .on_headers_complete = on_info,
            .on_message_complete = on_cinfo,
            .on_header_field = on_header_field,
            .on_header_value = on_header_value,
            .on_url = on_url,
            .on_status = on_status,
            .on_body = NULL,
    };

    llhttp_init(&parser, HTTP_REQUEST, &settings);
    parser.data = &zv_headers;
/* Use `llhttp_set_type(&parser, HTTP_REQUEST);` to override the mode */

    enum llhttp_errno err = llhttp_execute(&parser, headers, len);
    if (err == HPE_OK) {
        /* Successfully parsed! */

        printf("%s  \n", llhttp_method_name(parser.method));
        struct uri_parsed *parsed = parse_querystring(querystring);
        array_init_size(&zv_qs, parsed->qs_size);
        for (int i = 0; i < parsed->qs_size; ++i) {
            printf("key %s\n", parsed->get_qs[i].key);
            printf("value %s\n", parsed->get_qs[i].value);
            add_assoc_string(&zv_qs, parsed->get_qs[i].key, parsed->get_qs[i].value);
        }

        char version[5] = "\0";
        sprintf(version, "%d.%d", parser.http_major, parser.http_minor);

        zend_update_property_string(request->ce, request, PROP("method"), llhttp_method_name(parser.method));
        zend_update_property_string(request->ce, request, PROP("querystring"), querystring);
        zend_update_property_string(request->ce, request, PROP("HttpVersion"), version);
        zend_update_property_string(request->ce, request, PROP("uri"), parsed->uri);
        zend_update_property(request->ce, request, PROP("headers"), &zv_headers);
        zend_update_property(request->ce, request, PROP("query"), &zv_qs);
        printf("parse finished");
//        printf("query string is %s %s", querystring, version);
        efree(header_name);
        efree(querystring);
    } else {
        fprintf(stderr, "Parse error: %s %s\n", llhttp_errno_name(err),
                parser.reason);
    }
}
