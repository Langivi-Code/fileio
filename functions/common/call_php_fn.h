//
// Created by admin on 12.12.2021.
//

#include "callback_interface.h"

#ifndef FILEIO_CALL_PHP_FN_H
#define FILEIO_CALL_PHP_FN_H
void call_php_fn(uv_cb_type cb, size_t arg_count, zval * args, zval * retval, char * fun_name);
#endif //FILEIO_CALL_PHP_FN_H
