//
// Created by admin on 14.09.2021.
//


#include <assert.h>
#include <php.h>
#include "../common/callback_interface.h"
#include "../../php_fileio.h"
#include "file_interface.h"

#define LOG_TAG "file_put_contents_async * "

void on_write(uv_fs_t *req);

void on_wr_open(uv_fs_t *req);

/* {{{ void file_put_contents_async() */
PHP_FUNCTION (file_put_contents_async) {
    char *filename;
    size_t filename_len;
    zval * data;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    zend_long flags = 0;
    /* Argument to pass to function */
//    /* Parse arguments */
    ZEND_PARSE_PARAMETERS_START(2, 4)
            Z_PARAM_PATH(filename, filename_len)
            Z_PARAM_ZVAL(data)
            Z_PARAM_OPTIONAL
            Z_PARAM_FUNC(fci, fcc)
            Z_PARAM_LONG(flags)ZEND_PARSE_PARAMETERS_END();

    file_handle_data *file_data_handle = emalloc(sizeof(file_handle_data));
    memset(file_data_handle, 0, sizeof(file_handle_data));
    uv_fs_t *open_req = emalloc(sizeof(uv_fs_t));
    fs_id_t *fs_id = emalloc(sizeof(fs_id_t));
    printf("File name to write: %s\n", filename);

    init_cb(&fci, &fcc, &file_data_handle->php_cb_data);

    file_data_handle->filename = filename;
    if (Z_TYPE_P(data) == IS_STRING) {
        size_t data_size = Z_STRLEN_P(data);
        file_data_handle->buffer = uv_buf_init(emalloc(sizeof(char) * data_size), data_size);
        memset(file_data_handle->buffer.base, 0, data_size);
        strncpy(file_data_handle->buffer.base, ZSTR_VAL(Z_STR_P(data)), data_size);
        file_data_handle->file_size = data_size;
    } else {
        zend_argument_value_error(2, "must be a string");
        RETURN_THROWS();
    }
    file_data_handle->read = false;
    fs_id->id = add_fs_handle(fs_handle_map, file_data_handle);
    file_data_handle->close_requests.open_req = open_req;
    printf("%llu\n", fs_id->id);
    open_req->data = fs_id;
    int r = uv_fs_open(MODULE_GL(loop), open_req, filename, O_WRONLY | O_CREAT, S_IRWXU, on_wr_open);
    if (r) {
        fprintf(stderr, "Error at opening file: %s.\n",
                uv_strerror(r));
    }
    RETURN_BOOL(1);

//    if (maxlen_is_null) {
//        maxlen = (ssize_t) PHP_STREAM_COPY_ALL;
//    } else if (maxlen < 0) {
//        zend_argument_value_error(5, "must be greater than or equal to 0");
//        RETURN_THROWS();
//    }
//    int error = uv_fs_open(fileio_globals.loop, (uv_fs_t *) &uv->uv.fs, filename, O_RDONLY, S_IRUSR, php_uv_fs_cb);
//    if (error) {
//        PHP_UV_DEINIT_UV(uv);
//        php_error_docref(NULL, E_WARNING, "uv_open failed");
//        return;
//        fci.retval = &retval;
//        fci.param_count = 1;
//        fci.params = &arg;
//        int ret;
//
//        //OR
//        ZVAL_COPY(&arg, zv);
//        ret = zend_call_function(&fci, &fcc);
//        i_zval_ptr_dtor(&arg);
//        if (ret != SUCCESS || Z_TYPE(retval) == IS_UNDEF) {
//            zend_array_destroy(Z_ARR_P(return_value));
//            RETURN_NULL();
//        }
//
//        //OR
//        if (zend_call_function(&fci, &fcc) == SUCCESS && Z_TYPE(retval) != IS_UNDEF) {
//            if (EXPECTED(Z_TYPE(retval) == IS_STRING)) {
//                contents = Z_STR(retval);
//            } else {
//                contents = zval_get_string_func(&retval);
//                zval_ptr_dtor(&retval);
//            }
//        } else {
//            if (!EG(exception)) {
//                php_error_docref(NULL, E_WARNING, "Unable to call custom replacement function");
//            }
//        }
//        context = php_stream_context_from_zval(zcontext, 0);
//
//        stream = php_stream_open_wrapper_ex(filename, "rb",
//                                            (use_include_path ? USE_PATH : 0) | REPORT_ERRORS,
//                                            NULL, context);
//        if (!stream) {
//            RETURN_FALSE;
//        }
//
//        if (offset != 0 && php_stream_seek(stream, offset, ((offset > 0) ? SEEK_SET : SEEK_END)) < 0) {
//            php_error_docref(NULL, E_WARNING, "Failed to seek to position " ZEND_LONG_FMT " in the stream", offset);
//            php_stream_close(stream);
//            RETURN_FALSE;
//        }
//
//        if ((contents = php_stream_copy_to_mem(stream, maxlen, 0)) != NULL) {
//            RETVAL_STR(contents);
//        } else {
//            RETVAL_EMPTY_STRING();
//        }
//
//        php_stream_close(stream);
//
//        php_printf("The extension %s is loaded and working!\r\n", "fileio");
//    }
}

void on_wr_open(uv_fs_t *req) {

    // The request passed to the callback is the same as the one the call setup
    // function was passed.

    fs_id_t *fs_id = (fs_id_t *) req->data;
    LOG("file id is %zd\n", req->result);
    fs_id_item_t *fs_handle = find_fs_handle(fs_handle_map, fs_id->id);
    uv_fs_t *write_req = emalloc(sizeof(uv_fs_t));
    write_req->data = fs_id;
    fs_handle->handle->close_requests.write_req = write_req;
    assert(req == fs_handle->handle->close_requests.open_req);
    if (req->result >= 0) {
        fs_handle->handle->file = req->result;
        printf("%s\n", fs_handle->handle->buffer.base);
        uv_fs_write(MODULE_GL(loop), write_req, fs_handle->handle->file, &fs_handle->handle->buffer, 1, 0, on_write);
    } else {
        fprintf(stderr, "error opening file: %s\n", uv_strerror((int) req->result));
    }
}

void on_write(uv_fs_t *req) {
    fs_id_t *fs_id = (fs_id_t *) req->data;
    fs_id_item_t *fs_handle = find_fs_handle(fs_handle_map, fs_id->id);
    LOG("file idd is %d\n", fs_handle->handle->file);
    uv_fs_t *close_req = emalloc(sizeof(uv_fs_t));
    fs_handle->handle->close_requests.write_req = req;
    fs_handle->handle->close_requests.read_req = NULL;
    fs_handle->handle->close_requests.status_req = NULL;
    close_req->data = fs_id;

    if (req->result < 0) {
        fprintf(stderr, "Write error: %s\n", uv_strerror((int) req->result));
    } else {
        if (req->result > 0) {
            LOG(" Call back result %llu", fn_fs(req));
        }
        uv_fs_close(MODULE_GL(loop), close_req, fs_handle->handle->file, close_cb);
    }
}
