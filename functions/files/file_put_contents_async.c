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
    zval *data;
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
            Z_PARAM_LONG(flags)
    ZEND_PARSE_PARAMETERS_END();

    uv_fs_t *open_req = emalloc(sizeof(uv_fs_t));
    uv_fs_t *write_req = emalloc(sizeof(uv_fs_t));
    printf("File name to write: %s\n", filename);
    file_handle_data *handleData = emalloc(sizeof(file_handle_data));
    fill_file_handle(handleData, filename, &fci, &fcc);
    if (Z_TYPE_P(data) == IS_STRING) {
        size_t data_size = Z_STRLEN_P(data);
        handleData->buffer = uv_buf_init(malloc(sizeof(char) * (data_size+1)), data_size+1);
        memset(handleData->buffer.base, '\0', data_size + 1);
        strncpy(handleData->buffer.base, ZSTR_VAL(Z_STR_P(data)), data_size);
        handleData->file_size = data_size;
    } else{
        zend_argument_value_error(2, "must be a string");
        RETURN_THROWS();
    }
    handleData->read = false;

    fill_fs_handle_with_data(write_req, handleData);
    unsigned long id = add_fs_handle(write_req);
    printf("%lu\n", id);
    open_req->data = (void *) id;
    int r = uv_fs_open(MODULE_GL(loop), open_req, filename, O_WRONLY | O_CREAT, 0, on_wr_open);
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
    printf("file id is %zd\n", req->result);

//    assert(req == &open_req);
    if (req->result >= 0) {
        uv_fs_t *write_req = find_fs_handle((unsigned long long) req->data)->open_req;
        file_handle_data *handle = (file_handle_data *) write_req->data;
        handle->file = req->result;
        handle->handle_id = (unsigned long long) req->data;
        printf("%s\n", handle->buffer.base);
        uv_fs_write(MODULE_GL(loop), write_req, handle->file, &handle->buffer, 1, -1, on_write);
    } else {
        fprintf(stderr, "error opening file: %s\n", uv_strerror((int) req->result));
    }
    uv_fs_req_cleanup(req);
}

void on_write(uv_fs_t *req) {
    file_handle_data *handle = (file_handle_data *) req->data;
    uv_fs_t close_req;
    fs_close_reqs_t *requests = emalloc(sizeof(fs_close_reqs_t));
    requests->write_req = req;
    requests->open_req = handle->open_req;
    requests->read_req = NULL;
    close_req.data = (void *) requests;

    if (req->result < 0) {
        fprintf(stderr, "Write error: %s\n", uv_strerror((int) req->result));
    } else {
        if (req->result > 0) {
            LOG(" Call back result %llu", fn_fs(req));
        }
        uv_fs_close(MODULE_GL(loop), &close_req, handle->file, close_cb);
    }
}
