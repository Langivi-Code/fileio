//
// Created by user on 22.10.21.
//

#include "../../3rd/llhttp/llhttp.h"
#include "header.h"
#include "SAPI.h"
#include "../../fileio_arginfo.h"
#include <php.h>
#include <zend_API.h>
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
char *querystring = NULL;

static int on_status(llhttp_t *p, const char *at, size_t length) {
    printf("STATUS IS is %s\n", llhttp_method_name(p->method));
    printf("at %s %d\n", at, length);
    return 0;
}

static int on_url(llhttp_t *p, const char *at, size_t length) {
    printf("METHOD IS is %s\n", llhttp_method_name(p->method));
    querystring = emalloc(sizeof(char) * (length + 1));
    memset(querystring, 0, sizeof(char) * (length + 1));
    strncpy(querystring, at, length);
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

void parse(char *headers, size_t len, zend_object *request) {
    llhttp_t parser;
//    llhttp_settings_init(&settings);
    zval zv_headers;
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
        zend_update_property_string(request->ce, request, PROP("method"), llhttp_method_name(parser.method));
        zend_update_property_string(request->ce, request, PROP("query"), querystring);
        char version[5] = "\0";
        sprintf(version, "%d.%d", parser.http_major, parser.http_minor);
        zend_update_property(request->ce, request, PROP("HttpVersion"), version);
        zend_update_property(request->ce, request, PROP("headers"), &zv_headers);
        printf("parse finished");
        printf("query string is %s %s", querystring, version);
        efree(header_name);
    } else {
        fprintf(stderr, "Parse error: %s %s\n", llhttp_errno_name(err),
                parser.reason);
    }
}

void parse_querystring(const char *querystring) {

    char array[] = "address=1234&port=1234&username=1234&password=1234"
                   "&gamename=1234&&square=1234&LOGIN=LOGIN",
            *query = strdup(array),  /* duplicate array, &array is not char** */
    *tokens = query,
            *p = query;

    while ((p = strsep(&tokens, "&\n"))) {
        char *var = strtok(p, "="),
                *val = NULL;
        if (var && (val = strtok(NULL, "=")))
            printf("%-8s    %s\n", var, val);
        else
            fputs("<empty field>\n", stderr);
    }

    free(query);
}