//
// Created by admin on 14.09.2021.
//

#include <uv.h>
#include <php.h>
#include <zend_API.h>
#include "../files/file_interface.h"
#include "callback_interface.h"

void fn(uv_timer_t *handle) {
    printf("something");
    uv_cb_type *uv = (uv_cb_type *) handle->data;
    printf(" %lu \n", sizeof uv->fci);
    //    memcpy(&uv, (uv_cb_t *) handle->data, sizeof(uv_cb_t));
    zend_long error;
    zval retval;
    zval dstr;
    ZVAL_STRING(&dstr, "callback fn");
    //    zend_call_method_with_1_params(NULL, NULL, NULL, "print_r", &retval, &dstr);
    if (ZEND_FCI_INITIALIZED(uv->fci)) {
        printf("Timeout call back is called\n");
        if (zend_call_function(&uv->fci, &uv->fcc) != SUCCESS) {
            error = -1;
        }

    } else {
        error = -2;
    }
    efree(handle);
}

void fn_idle(uv_idle_t *handle) {
    uv_cb_type *uv = (uv_cb_type *) handle->data;
    printf(" %lu \n", sizeof uv->fci);
    //    memcpy(&uv, (uv_cb_t *) handle->data, sizeof(uv_cb_t));
    zend_long error;
    zval retval;
    zval dstr;
    ZVAL_STRING(&dstr, "callback fn");
    //    zend_call_method_with_1_params(NULL, NULL, NULL, "print_r", &retval, &dstr);
    if (ZEND_FCI_INITIALIZED(uv->fci)) {
        printf("Idle call back is called");
        if (zend_call_function(&uv->fci, &uv->fcc) != SUCCESS) {
            error = -1;
        }

    } else {
        error = -2;
    }
    uv_idle_stop(handle);
    efree(handle);
}

void fn_interval(uv_timer_t *handle) {
    uv_cb_type *uv = (uv_cb_type *) handle->data;
    printf(" %lu \n", sizeof uv->fci);
    //    memcpy(&uv, (uv_cb_t *) handle->data, sizeof(uv_cb_t));
    zend_long error;
    zval retval;
    zval dstr;
    ZVAL_STRING(&dstr, "callback fn");
    //    zend_call_method_with_1_params(NULL, NULL, NULL, "print_r", &retval, &dstr);
    if (ZEND_FCI_INITIALIZED(uv->fci)) {
        printf("Timeout call back is called\n");
        if (zend_call_function(&uv->fci, &uv->fcc) != SUCCESS) {
            error = -1;
        }
    } else {
        error = -2;
    }
}


void fn_fs(uv_fs_t *handle) {
    printf("something");
    file_handle_data *file_handle = (file_handle_data *) handle->data;
    printf(" %lu \n", sizeof file_handle->php_cb_data);
    //    memcpy(&uv, (uv_cb_t *) handle->data, sizeof(uv_cb_t));
    zend_long error;
    zval retval = {};
    zval dstr[1];
    ZVAL_STRINGL(&dstr[0], file_handle->buffer.base, file_handle->buffer.len);
    //    zval params[1];
//    ZVAL_COPY_VALUE(&params[0], &callable);
    file_handle->php_cb_data.fci.retval = &retval;
    file_handle->php_cb_data.fci.param_count = 1;
    file_handle->php_cb_data.fci.params = dstr;
    //    zend_call_method_with_1_params(NULL, NULL, NULL, "print_r", &retval, &dstr);
    if (ZEND_FCI_INITIALIZED(file_handle->php_cb_data.fci)) {
        printf("FS call back is called\n");
        if (zend_call_function(&file_handle->php_cb_data.fci, &file_handle->php_cb_data.fcc) != SUCCESS) {
            error = -1;
        }
    } else {
        error = -2;
    }
}