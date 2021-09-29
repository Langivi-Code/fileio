//
// Created by admin on 14.09.2021.
//

#include <uv.h>
#include <assert.h>
#include <php.h>
#include <zend_API.h>
#include "../common/callback_interface.h"
#include "../../php_fileio.h"
#include "file_interface.h"


void on_read(uv_fs_t *req);

static uv_buf_t iov;
static uv_fs_t open_req;
static uv_fs_t read_req;
static uv_fs_t status_req;
static uint64_t st_size;
static uv_file file;


void on_status(uv_fs_t *req) {
    if (req->result == 0) {
        st_size = req->statbuf.st_size;
        printf("file size is %lu\n", st_size);
        printf("starting reading ... %d\n", file);
        iov = uv_buf_init(malloc(sizeof(char) * st_size), st_size);
        uv_fs_read(FILE_IO_GLOBAL(loop), &read_req, file,
                   &iov, 1, -1, on_read);
    }
}

void on_open(uv_fs_t *req) {
    // The request passed to the callback is the same as the one the call setup
    // function was passed.
    printf("file id is %zd\n", req->result);

    assert(req == &open_req);
    if (req->result >= 0) {
        file = req->result;
        uv_fs_fstat(FILE_IO_GLOBAL(loop), &status_req, req->result, on_status);
    } else {
        fprintf(stderr, "error opening file: %s\n", uv_strerror((int) req->result));
    }
}

void on_read(uv_fs_t *req) {
    uv_fs_t close_req;
    if (req->result < 0) {
        fprintf(stderr, "Read error: %s\n", uv_strerror(req->result));
    } else if (req->result == 0) {

        // synchronous
        uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);
    } else if (req->result > 0) {

        iov.len = req->result;
        fn_fs(req, iov.base,  iov.len);
//        uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);
//        printf("%s",dest);
    }
}


/* {{{ void file() */
PHP_FUNCTION (file_get_contents_async) {
    char *filename;
    size_t filename_len;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    zend_long offset = 0;

//    zend_string * contents;
          /* Function return value */
//    zval arg;                /* Argument to pass to function */
//    /* Parse arguments */
    ZEND_PARSE_PARAMETERS_START(2, 3)
            Z_PARAM_PATH(filename, filename_len)
            Z_PARAM_FUNC(fci, fcc)
            Z_PARAM_OPTIONAL
            Z_PARAM_LONG(offset)
    ZEND_PARSE_PARAMETERS_END();
    printf("%s", filename);
    file_handle_data * handleData = emalloc(sizeof(file_handle_data));
    handleData->filename = filename;
    fill_fs_handle_with_data(&read_req, &fci, &fcc);
    int r = uv_fs_open(uv_default_loop(), &open_req, filename, O_RDONLY, 0, on_open);
    if (r) {
//        fprintf(stderr, "Error at opening file: %s.\n",
//                uv_strerror(uv_last_error(uv_default_loop())));
    }

    uv_fs_req_cleanup(&open_req);
    uv_fs_req_cleanup(&read_req);
    uv_fs_req_cleanup(&status_req);
//    uv_fs_req_cleanup(&write_req);
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