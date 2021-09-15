//
// Created by admin on 14.09.2021.
//

#include <uv.h>
#include <php.h>
#include <zend_API.h>
#include "callback_interface.h"

void fn(uv_timer_t *handle) {
    uv_cb_type * uv = (uv_cb_type *) handle->data;
    //    memcpy(&uv, (uv_cb_t *) handle->data, sizeof(uv_cb_t));
    zend_long error;
    zval retval;
    zval dstr;
    ZVAL_STRING(&dstr,"callback fn");
    //    zend_call_method_with_1_params(NULL, NULL, NULL, "print_r", &retval, &dstr);
    if (ZEND_FCI_INITIALIZED(uv->fci)) {
        printf("Call back is called");
        if (zend_call_function(&uv->fci, &uv->fcc) != SUCCESS) {
            error = -1;
        }

    } else {
        error = -2;
    }
}