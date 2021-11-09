//
// Created by admin on 14.09.2021.
//

#include <uv.h>
#include <php.h>
#include <zend_API.h>
#include "../files/file_interface.h"
#include "callback_interface.h"

void clean_cb(uv_cb_type  *cb){
    if (ZEND_FCI_INITIALIZED(cb->fci)) {
        zval_dtor(&cb->fci.function_name);

        if (cb->fci.object != NULL) {
            OBJ_RELEASE(cb->fci.object);
        }
    }

    efree(cb);
    cb = NULL;
}

#define PHP_UV_CHECK_VALID_FD(fd, zstream) \
if (fd < 0) { \
php_error_docref(NULL, E_WARNING, "invalid variable passed. can't convert to fd."); \
PHP_UV_DEINIT_UV(uv); \
RETURN_FALSE; \
} \
if (Z_ISUNDEF(uv->fs_fd)) { \
ZVAL_COPY(&uv->fs_fd, zstream); \
}

void fn(uv_timer_t *handle) {
    #define LOG_TAG "fn"
    uv_cb_type *uv = (uv_cb_type *) handle->data;
    printf(" %lu \n", sizeof uv->fci);
    //    memcpy(&uv, (uv_cb_t *) handle->data, sizeof(uv_cb_t));
    zend_long error;
    zval retval;
    zval dstr;
    ZVAL_STRING(&dstr, "callback fn");
    //    zend_call_method_with_1_params(NULL, NULL, NULL, "print_r", &retval, &dstr);
    if (ZEND_FCI_INITIALIZED(uv->fci)) {
        LOG("Timeout call back is called\n");
        if (zend_call_function(&uv->fci, &uv->fcc) != SUCCESS) {
            error = -1;
        }

    } else {
        error = -2;
    }
    efree(handle);
}

void fn_idle(uv_idle_t *handle) {
    #define LOG_TAG "fn_idle"
    uv_cb_type *uv = (uv_cb_type *) handle->data;
    printf(" %lu \n", sizeof uv->fci);
    //    memcpy(&uv, (uv_cb_t *) handle->data, sizeof(uv_cb_t));
    zend_long error;

    zval dstr;
    ZVAL_STRING(&dstr, "callback fn");
    //    zend_call_method_with_1_params(NULL, NULL, NULL, "print_r", &retval, &dstr);

    if (ZEND_FCI_INITIALIZED(uv->fci)) {
        LOG("Idle call back is called");
        if (zend_call_function(&uv->fci, &uv->fcc) != SUCCESS) {
            error = -1;
        }

    } else {
        error = -2;
    }
    uv_idle_stop(handle);
//    efree(handle);
}

void fn_interval(uv_timer_t *handle) {
    #define LOG_TAG "fn_interval"
    uv_cb_type *uv = (uv_cb_type *) handle->data;
    printf(" %lu \n", sizeof uv->fci);
    //    memcpy(&uv, (uv_cb_t *) handle->data, sizeof(uv_cb_t));
    zend_long error;
    zval retval;
    zval dstr;
    ZVAL_STRING(&dstr, "callback fn");
    //    zend_call_method_with_1_params(NULL, NULL, NULL, "print_r", &retval, &dstr);
    if (ZEND_FCI_INITIALIZED(uv->fci)) {
        LOG("Timeout call back is called");
        if (zend_call_function(&uv->fci, &uv->fcc) != SUCCESS) {
            error = -1;
        }
    } else {
        error = -2;
    }
}


zend_long fn_fs(uv_fs_t *handle) {
#define LOG_TAG "fn_fs"
    file_handle_data *file_handle = (file_handle_data *) handle->data;
    LOG("size of php_cb_data: %lu", sizeof file_handle->php_cb_data);
    static short counter = 0;
    zend_long error = 0;
    zval retval = {};
    zval dest = {};
    zval dstr[1];
    counter++;
    char data[50];

    int length = sprintf(data, "Callback # %d  pointer %p", counter, &file_handle->php_cb_data.fci);
    char dd[file_handle->buffer.len + length];
    if (file_handle->read == true) {
        strcat(dd, data);
        strncat(dd, file_handle->buffer.base, file_handle->buffer.len);
        ZVAL_STRING(&dstr[0], &dd);
//        ZVAL_STRINGL(&dest, file_handle->buffer.base, file_handle->buffer.len);
//        ZVAL_COPY(&dstr[0],&dest);
    }
//    printf("%s",file_handle->buffer.base);
    file_handle->php_cb_data.fci.retval = &retval;
    file_handle->php_cb_data.fci.param_count = 1;
    file_handle->php_cb_data.fci.params = dstr;
    //    zend_call_method_with_1_params(NULL, NULL, NULL, "print_r", &retval, &dstr);
    if (ZEND_FCI_INITIALIZED(file_handle->php_cb_data.fci)) {

        LOG("FS call back is initalized");
        if (zend_call_function(&file_handle->php_cb_data.fci, &file_handle->php_cb_data.fcc) != SUCCESS) {
            error = -1;
        }
    } else {
        error = -2;
    }
    zval_ptr_dtor(&dstr[0]);
    zval_ptr_dtor(&retval);
    LOG("PHP Callback ended, status - %lld", error);
    return  error;
}
