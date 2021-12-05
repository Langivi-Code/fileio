//
// Created by admin on 30.10.2021.
//

#ifndef FILEIO_SERVER_ARGS_INFO_H
#define FILEIO_SERVER_ARGS_INFO_H
ZEND_BEGIN_ARG_INFO_EX(arginfo_server, 0,0,1)
ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, host, IS_STRING,  1, "null")
ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_server_event_handler, 0)
ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_server_write, 0)
                ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(arginfo_server_setBuffer, 0)
                ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif //FILEIO_SERVER_ARGS_INFO_H
