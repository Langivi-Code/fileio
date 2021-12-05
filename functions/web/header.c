//
// Created by user on 22.10.21.
//

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