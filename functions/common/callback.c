//
// Created by admin on 14.09.2021.
//

#include <uv.h>
#include <php.h>
#include <zend_API.h>
#include "../files/file_interface.h"
#include "callback_interface.h"
#include "call_php_fn.h"
#include "../../php_fileio.h"
#include "../timers/timers_interface.h"

void clean_cb(uv_cb_type *cb) {
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
    timerData *timer_data = (timerData *) handle->data;
    printf(" %lu \n", sizeof timer_data->cb.fci);
    //    memcpy(&uv, (uv_cb_t *) handle->data, sizeof(uv_cb_t));
    zend_long error;
    zval retval;
    zval dstr;
    ZVAL_STRING(&dstr, "callback fn");
    //    zend_call_method_with_1_params(NULL, NULL, NULL, "print_r", &retval, &dstr);
    if (ZEND_FCI_INITIALIZED(timer_data->cb.fci)) {
        LOG("Timeout call back is called\n");
        if (zend_call_function(&timer_data->cb.fci, &timer_data->cb.fcc) != SUCCESS) {
            error = -1;
        }

    } else {
        error = -2;
    }
    efree(handle);
    remove_handle(timer_data->handle_id);
    efree(timer_data);
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
    efree(uv->fci.params);
    uv_idle_stop(handle);
    efree(handle);
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

    fs_id_t *fs_id = (fs_id_t *) handle->data;
    fs_id_item_t *fs_handle = find_fs_handle(fs_handle_map, fs_id->id);
    file_handle_data *file_handle = fs_handle->handle;
    static short counter = 0;
    zval retval;
    zval args[1];
    counter++;

    if (file_handle->read == true) {
        char output_text[file_handle->buffer.len];
        memset(output_text, 0, file_handle->buffer.len);
//        strcat(dd, data);
        printf("char len %zu %lu\n", file_handle->buffer.len, strlen(file_handle->buffer.base));
        strncat(output_text, file_handle->buffer.base, file_handle->buffer.len);
        ZVAL_STRING(&args[0], &output_text);
    }
//    printf("%s",file_handle->buffer.base);

    LOG("FS call back is initalized");
    call_php_fn(&file_handle->php_cb_data, file_handle->read ? 1 : 0, file_handle->read ? args : NULL, &retval,
                "fn_fs");

    //    zend_call_method_with_1_params(NULL, NULL, NULL, "print_r", &retval, &dstr);
    uv_fs_t *close_req = emalloc(sizeof(uv_fs_t));
    close_req->data = fs_id;
    uv_fs_close(MODULE_GL(loop), close_req, handle->file, close_cb);
    puts("dats");
    if (file_handle->read == true) {
        zval_ptr_dtor(&args[0]);
    }
//    zval_ptr_dtor(&retval);
}
