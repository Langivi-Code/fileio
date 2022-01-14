//
// Created by admin on 12.12.2021.
//
#include <php.h>
#include <zend_exceptions.h>
#include <string.h>
#include "callback_interface.h"
#include "call_php_fn.h"
#include "../http/request.h"

void call_php_fn(uv_cb_type * cb, size_t arg_count, zval * args, zval * retval, char * fun_name) {
    zend_long error = 0;
    cb->fci.param_count = arg_count;
    cb->fci.params = args;
    cb->fci.retval = retval;
    cb->fcc.function_handler->common.function_name = zend_string_init(fun_name, strlen(fun_name), 1);
    if (ZEND_FCI_INITIALIZED(cb->fci)) {
        if (/*zend_call_function(&cb->fci, &cb->fcc)*/1 != SUCCESS) {
            error = -1;
            zend_function * fun = cb->fcc.function_handler;
            if (!EG(exception)) {
                zend_error_noreturn(E_CORE_ERROR, "Couldn't execute method %s%s%s",
                                    fun->common.scope ? ZSTR_VAL(fun->common.scope->name) : "",
                                    fun->common.scope ? "::" : "", ZSTR_VAL(fun->common.function_name));
            } else {
                zend_exception_error(EG(exception), E_WARNING);
                EG(exception) = NULL;
            }
            exit(-1);
        }
    } else {
        error = -2;
    }
}