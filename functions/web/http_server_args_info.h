//
// Created by admin on 30.10.2021.
//

#ifndef FILEIO_SERVER_ARGS_INFO_H
#define FILEIO_SERVER_ARGS_INFO_H

ZEND_BEGIN_ARG_INFO_EX(arginfo_http_server, 0, 0, 1)
                ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
                ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, host, IS_STRING, 1, "null")
                ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "[]")
                ZEND_ARG_TYPE_INFO(0, onConnect, IS_CALLABLE, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_http_server_event_handler, 0)
                ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_http_server_write, 0)
                ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(arginfo_http_server_setBuffer, 0)
                ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_INFO(arginfo_http_server_set_status_code, 0)
                ZEND_ARG_TYPE_INFO(0, code, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_http_server_set_header, 0, 2, HttpResponse, MAY_BE_OBJECT)
                ZEND_ARG_TYPE_INFO(0, headerName, IS_STRING, 0)
                ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(arginfo_http_server_set_public_path, 0)
                ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif //FILEIO_SERVER_ARGS_INFO_H
