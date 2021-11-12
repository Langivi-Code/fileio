//
// Created by user on 22.10.21.
//

#include "../../3rd/llhttp/llhttp.h"
#include "header.h"
#include "SAPI.h"
#include "../../fileio_arginfo.h"
#include <php.h>
#include <zend_API.h>

PHP_FUNCTION (send_header) {
    zend_string *key, *value;
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


static int on_info(llhttp_t* p) {
    printf("some info COMPLITED\n\n");
    return 0;
}


static int on_cinfo(llhttp_t* p) {
    printf("PARSE COMPLITED");
    return 0;
}
static int on_status(llhttp_t* p, const char *at, size_t length) {
    printf("STATUS IS is %s\n", llhttp_method_name(p->method));
    printf("at %s %d\n",at, length );
    return 0;
}
static int on_url(llhttp_t* p, const char *at, size_t length) {
    printf("URL IS is %s\n", llhttp_method_name(p->method));
    printf("at %s %d\n",at, length );
    return 0;
}


static int on_data(llhttp_t* p, const char *at, size_t length) {
    printf("method is %s\n", llhttp_method_name(p->method));
    printf("header is %s %d\n\n\n",at, length );
    return 0;
}
static int on_hdata(llhttp_t* p, const char *at, size_t length) {
    printf("method is %s\n", llhttp_method_name(p->method));
    printf("header value is %s %d\n\n\n",at, length );
    return 0;
}

void parse(char * headers, size_t len) {
    llhttp_t parser;
//    llhttp_settings_init(&settings);
    llhttp_settings_t settings = {
            .on_message_begin = on_info,
            .on_headers_complete = on_info,
            .on_message_complete = on_cinfo,
            .on_header_field = on_data,
            .on_header_value = on_hdata,
            .on_url = on_url,
            .on_status = on_status,
            .on_body = on_data
    };

    llhttp_init(&parser, HTTP_REQUEST, &settings);

/* Use `llhttp_set_type(&parser, HTTP_REQUEST);` to override the mode */

    enum llhttp_errno err = llhttp_execute(&parser, headers, len);
    if (err == HPE_OK) {
        /* Successfully parsed! */
        printf("parse finished");
    } else {
        fprintf(stderr, "Parse error: %s %s\n", llhttp_errno_name(err),
                parser.reason);
    }
}