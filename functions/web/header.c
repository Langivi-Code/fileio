//
// Created by user on 22.10.21.
//

#include "../../3rd/llhttp/llhttp.h"
#include "header.h"
#include "SAPI.h"
#include "../../fileio_arginfo.h"
#include <php.h>
#include <zend_API.h>
#include <php_variables.h>
#include "../http/request.h"
#include "../../3rd/utils/strpos.h"
#include "ext/standard/php_var.h"
#include "../common/mem.h"
#include "zend_smart_string.h"

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

static int on_status(llhttp_t *p, const char *at, size_t length) {
    printf("STATUS IS is %s\n", llhttp_method_name(p->method));
    printf("at %s %d\n", at, length);
    return 0;
}

static int on_url(llhttp_t *p, const char *at, size_t length) {
    struct input_data *data = p->data;
    data->qs = emalloc(sizeof(char) * (length + 1));
    memset(data->qs, 0, length + 1);
    strncpy(data->qs, at, length);
    printf("QS AFTER copy is %zu  %s\n", length, data->qs);
    return 0;
}


static int on_header_field(llhttp_t *p, const char *at, size_t length) {
    struct input_data *data = (struct input_data *) p->data;
    data->cur_header = emalloc(sizeof(char) * (length + 1));
    memset(data->cur_header, 0, length + 1);
    strncpy(data->cur_header, at, length);
    add_assoc_string(&data->headers, data->cur_header, "");

    return 0;
}

static int on_header_value(llhttp_t *p, const char *at, size_t length) {
    struct input_data *data = (struct input_data *) p->data;
    char *value = emalloc(sizeof(char) * (length + 1));
    memset(value, 0, (length + 1));
    strncpy(value, at, length);
    add_assoc_string(&data->headers, data->cur_header, value);
    efree(value);
    efree(data->cur_header);
    return 0;
}


void parse(char *headers, size_t len, zend_object *request) {
    llhttp_t parser;
//    llhttp_settings_init(&settings);
    struct input_data *data = emalloc(sizeof(struct input_data));
    zval zv_headers;
    data->headers = zv_headers;
    data->headers = zv_headers;
    array_init(&data->headers); //  zend_read_property(request->ce, request, PROP("headers"), 0, NULL);
//    mem("before parse headers");
    llhttp_settings_t settings = {
            .on_message_begin = NULL,
//            .on_headers_complete = on_info,
//            .on_message_complete = on_cinfo,
            .on_header_field = on_header_field,
            .on_header_value = on_header_value,
            .on_url = on_url,
            .on_status = on_status,
            .on_body = NULL,
    };

    llhttp_init(&parser, HTTP_REQUEST, &settings);
    parser.data = data;
/* Use `llhttp_set_type(&parser, HTTP_REQUEST);` to override the mode */

    enum llhttp_errno err = llhttp_execute(&parser, headers, len);
    if (err == HPE_OK) {
        /* Successfully parsed! */
        printf("%s  \n", llhttp_method_name(parser.method));
        char scpy[strlen(data->qs) + 1];
        strcpy(scpy, data->qs);

        url_decode(scpy, strlen(scpy));
        printf("query string is %s %s\n", data->qs, scpy);
//        mem("before qs headers");
        struct uri_parsed * parsed = parse_urlstring(scpy);
//        mem("after qs headers");
        printf(" GET size is %zu\n",  parsed->get_qs.size);
        for (int i = 0; i < parsed->get_qs.size; ++i) {
            printf("key %s\n", parsed->get_qs.kv[i].key);
            printf("value %s\n", parsed->get_qs.kv[i].value);
            php_register_variable_safe(parsed->get_qs.kv[i].key,parsed->get_qs.kv[i].value, strlen(parsed->get_qs.kv[i].value), &PG(http_globals)[TRACK_VARS_GET]);
        }
//        mem("after qs assign");
        char version[5] = "\0";
        sprintf(version, "%d.%d", parser.http_major, parser.http_minor);

        zend_update_property_string(request->ce, request, PROP("method"), llhttp_method_name(parser.method));
        zend_update_property_string(request->ce, request, PROP("querystring"), scpy);
        zend_update_property_string(request->ce, request, PROP("HttpVersion"), version);
        zend_update_property_string(request->ce, request, PROP("uri"), parsed->uri);
        zend_update_property(request->ce, request, PROP("headers"), &data->headers);
        zend_update_property(request->ce, request, PROP("query"), &PG(http_globals)[TRACK_VARS_GET]);
//        printf("parse finished");
        efree(data->qs);
        efree(parsed->get_qs.kv);
        efree(parsed);
        efree(data);
//        mem("after free headers");
    } else {
        fprintf(stderr, "Parse error: %s %s\n", llhttp_errno_name(err),
                parser.reason);
    }
}

smart_string *stringify(key_value_collection collection) {
    smart_string * res = emalloc(sizeof (smart_string));
    memset(res, 0, sizeof (smart_string));
    smart_string_appends(res, "");
    if (collection.size == 0) {
        return res;
    }

    for (int i = 0; i < collection.size; i++) {
        smart_string_appends(res, collection.kv[i].key);
        smart_string_appends(res, ": ");
        smart_string_appends(res, collection.kv[i].value);
        smart_string_appends(res, "\r\n");
    }
    return res;
}