//
// Created by admin on 12.12.2021.
//
#include <php.h>
#include <zend_API.h>
#include "reqest_class.h"
#include "../../php_fileio.h"
#include "../common/register_property.h"

#ifndef PROP
#define PROP(string)  string, sizeof(string) - 1
#endif

static const zend_function_entry class_ServerRequest_methods[] = {
        PHP_FE_END
};

zend_class_entry *register_class_HttpRequest(void) {
    zend_class_entry ce;
    short flags_pb_ro = ZEND_ACC_PUBLIC | ZEND_ACC_HAS_TYPE_HINTS;
    INIT_CLASS_ENTRY(ce, "HttpRequest", class_ServerRequest_methods);
    FILE_IO_GLOBAL(http_request_class) = zend_register_internal_class_ex(&ce, NULL);
    FILE_IO_GLOBAL(http_request_class)->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

    zval method;
    ZVAL_UNDEF(&method);
    register_property(FILE_IO_GLOBAL(http_request_class), PROP("method"), &method, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY,
                      MAY_BE_STRING);
    zval HttpVersion;
    ZVAL_UNDEF(&HttpVersion);
    register_property(FILE_IO_GLOBAL(http_request_class), PROP("HttpVersion"), &HttpVersion,
                      flags_pb_ro, MAY_BE_STRING);
    zval uri;
    ZVAL_UNDEF(&uri);
    register_property(FILE_IO_GLOBAL(http_request_class), PROP("uri"), &uri,
                      flags_pb_ro, MAY_BE_STRING);
    zval querystring;
    ZVAL_UNDEF(&querystring);
    register_property(FILE_IO_GLOBAL(http_request_class), PROP("querystring"), &uri,
                      flags_pb_ro, MAY_BE_STRING);
    zval body;
    ZVAL_UNDEF(&body);
    register_property(FILE_IO_GLOBAL(http_request_class), PROP("body"), &body, flags_pb_ro,
                      MAY_BE_ARRAY);
    zval headers;
    ZVAL_UNDEF(&headers);
    register_property(FILE_IO_GLOBAL(http_request_class), PROP("headers"), &headers, flags_pb_ro,
                      MAY_BE_ARRAY);
    zval query;
    ZVAL_UNDEF(&query);
    register_property(FILE_IO_GLOBAL(http_request_class), PROP("query"), &query, flags_pb_ro,
                      MAY_BE_ARRAY);


//    zend_declare_property_string(FILE_IO_GLOBAL(http_request_class), PROP("method"), "", flags_pb_ro);
//    zend_declare_property_string(FILE_IO_GLOBAL(http_request_class), PROP("HttpVersion"), "", flags_pb_ro);
//    zend_declare_property_string(FILE_IO_GLOBAL(http_request_class), PROP("uri"), "", flags_pb_ro);
//    zend_declare_property_string(FILE_IO_GLOBAL(http_request_class), PROP("querystring"), "", flags_pb_ro);
//    zend_declare_property_string(FILE_IO_GLOBAL(http_request_class), PROP("body"), "", flags_pb_ro);

//    zend_declare_property(FILE_IO_GLOBAL(http_request_class), PROP("headers"), &ht, flags_pb_ro);
//    zval query;
//    ZVAL_EMPTY_ARRAY(&query);
//    zend_declare_property(FILE_IO_GLOBAL(http_request_class), PROP("query"), &query, flags_pb_ro);

    return FILE_IO_GLOBAL(http_request_class);
}