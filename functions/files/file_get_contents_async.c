//
// Created by admin on 14.09.2021.
//

#include <uv.h>
#include <assert.h>
#include <php.h>
#include "stdio.h"
#include <zend_API.h>

#include "../common/callback_interface.h"
#include "../../php_fileio.h"
#include "file_interface.h"

#define LOG_TAG "file_get_contents_async"

static void on_read(uv_fs_t *req);

static void on_open(uv_fs_t *req);

\

/* {{{ void file() */
PHP_FUNCTION (file_get_contents_async) {
    char *filename;
    size_t filename_len;
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    zend_long offset = 0;
    bool maxlen_is_null = 1;
    zend_long maxlen = 0;

//TODO add check on file existance

//    zend_string * contents;
    /* Function return value */
//    zval arg;                /* Argument to pass to function */
//    /* Parse arguments */
    ZEND_PARSE_PARAMETERS_START(2, 4)
            Z_PARAM_PATH(filename, filename_len)
            Z_PARAM_FUNC(fci, fcc)
            Z_PARAM_OPTIONAL
            Z_PARAM_LONG(offset)
            Z_PARAM_LONG_OR_NULL(maxlen, maxlen_is_null)ZEND_PARSE_PARAMETERS_END();
    file_handle_data *handleData = emalloc(sizeof(file_handle_data));
    uv_fs_t *open_req = emalloc(sizeof(uv_fs_t));
    uv_fs_t *read_req = emalloc(sizeof(uv_fs_t));
    LOG("File name to read: %s %p %p\n", filename, &fci, handleData);

    fill_file_handle(handleData, filename, &fci, &fcc);
    fill_fs_handle_with_data(read_req, handleData);
    if (!maxlen_is_null) {
        handleData->file_size = maxlen;
    } else {
        handleData->file_size = 0;
    }
    handleData->read = true;

    unsigned short handle_count = count_fs_handles();
    unsigned long id = add_fs_handle(read_req);
    if (handle_count > 0) {
        LOG("ADD HANDLE %u\n", memcmp(&fstimeout_handle_map[handle_count - 1], &fstimeout_handle_map[handle_count],
                                      sizeof(fs_handles_id_item_t)));
        LOG("ADD FD HANDLE %u\n", memcmp(&
                                                 (
                                                         (file_handle_data *) fstimeout_handle_map[handle_count -
                                                                                                   1].open_req->data
                                                 )->php_cb_data.fci, &fci, sizeof(zend_fcall_info)));
    }

    LOG("Handle time id %lu\n", id);
    open_req->data = (void *) id;
    int r = uv_fs_open(FILE_IO_GLOBAL(loop), open_req, filename, O_RDONLY, 0, on_open);
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

void on_status(uv_fs_t *req) {
    if (req->result == 0) {
        uv_fs_t *read_req = find_fs_handle((unsigned long long) req->data)->open_req;
        file_handle_data *handle = (file_handle_data *) read_req->data;
        if (handle->file_size == 0) {
            handle->file_size = req->statbuf.st_size;
        }
        LOG("file size is %lu %lu\n", handle->file_size, req->statbuf.st_size);

        LOG("starting reading ... %d\n", handle->file);
        handle->buffer = uv_buf_init(malloc(sizeof(char) * (handle->file_size + 1)), handle->file_size + 1);
        memset(handle->buffer.base, '\0', handle->file_size + 1);
        uv_fs_read(FILE_IO_GLOBAL(loop), read_req, handle->file,
                   &handle->buffer, 1, -1, on_read);
    }
}

static void on_open(uv_fs_t *req) {
    // The request passed to the callback is the same as the one the call setup
    // function was passed.
    LOG("file id is %zd\n", req->result);

    uv_fs_t *status_req = emalloc(sizeof(uv_fs_t));
    assert(req == &open_req);
    if (req->result >= 0) {
        uv_fs_t *read_req = find_fs_handle((unsigned long long) req->data)->open_req;
        file_handle_data *handle = (file_handle_data *) read_req->data;
        handle->file = req->result;
        handle->handle_id = (unsigned long long) req->data;
        status_req->data = req->data;
        handle->open_req = req;
        uv_fs_fstat(FILE_IO_GLOBAL(loop), status_req, req->result, on_status);
    } else {
        fprintf(stderr, "error opening file: %s\n", uv_strerror((int) req->result));
    }
}

static void on_read(uv_fs_t *req) {
    uv_fs_t close_req;
    file_handle_data *handle = (file_handle_data *) req->data;
    fs_close_reqs_t *requests = emalloc(sizeof(fs_close_reqs_t));
    requests->write_req = NULL;
    requests->open_req = handle->open_req;
    requests->read_req = req;
    close_req.data = (void *) requests;
//    uv_fs_close(FILE_IO_GLOBAL(loop), &close_req, handle->file, close_cb);
    if (req->result < 0) {
        fprintf(stderr, "Read error: %s\n", uv_strerror(req->result));
    } else if (req->result == 0) {
        // synchronous
    } else if (req->result > 0) {
        fn_fs(req);
    }


}