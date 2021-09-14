/* fileio extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/file.h"
#include "php_fileio.h"
#include "zend_API.h"
#include "fiber.h"
#include "zend_closures.h"
#include "zend_interfaces.h"
#include "fileio_arginfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <wchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "uv.h"
#include <threads.h>


//extern PHP_FIBER_API zend_class_entry *zend_ce_fiber;
/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
    ZEND_PARSE_PARAMETERS_START(0, 0) \
    ZEND_PARSE_PARAMETERS_END()
#endif

/* {{{ void file_get_contents_async() */
//PHP_FUNCTION (file_get_contents_async) {
//    char *filename;
//    size_t filename_len;
//    zend_bool use_include_path = 0;
//    php_stream *stream;
//    zend_long offset = 0;
//    zend_long maxlen;
//    zend_bool maxlen_is_null = 1;
//    zval * zcontext = NULL;
//    php_stream_context *context = NULL;
//    zend_string * contents;
//    zend_fcall_info fci;
//    zend_fcall_info_cache fcc;
//    zval retval;            /* Function return value */
//    zval arg;                /* Argument to pass to function */
//    /* Parse arguments */
//    ZEND_PARSE_PARAMETERS_START(1, 6)
//            Z_PARAM_PATH(filename, filename_len)
//            Z_PARAM_FUNC(fci, fcc)
//            Z_PARAM_OPTIONAL
//            Z_PARAM_BOOL(use_include_path)
//            Z_PARAM_RESOURCE_OR_NULL(zcontext)
//            Z_PARAM_LONG(offset)
//            Z_PARAM_LONG_OR_NULL(maxlen, maxlen_is_null)ZEND_PARSE_PARAMETERS_END();
//
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
//}

/* }}} */
typedef struct {
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
} uv_cb_t;

thrd_t thrd;
uv_async_t as_h;


static void fn(uv_timer_t *handle) {
    uv_cb_t * uv =(uv_cb_t *) handle->data;
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

void fill_handle_with_data(
        uv_timer_t *handle,
        zend_fcall_info *fci,
        zend_fcall_info_cache *fcc
) {
    uv_cb_t uv = {};
    printf("size %lu\n", sizeof handle->data);
    printf(" %lu \n", sizeof fci);
//    uv.fci = (zend_fcall_info *)emalloc(sizeof(zend_fcall_info));
    handle->data = (uv_cb_t *) emalloc(sizeof(uv_cb_t));
    memcpy(&uv.fci, fci, sizeof(zend_fcall_info));
    memcpy(&uv.fcc, fcc, sizeof(zend_fcall_info_cache));
    memcpy(handle->data, &uv, sizeof(uv_cb_t));

    if (ZEND_FCI_INITIALIZED(*fci)) {
        Z_TRY_ADDREF(uv.fci.function_name);
        if (fci->object) {
            GC_ADDREF(fci->object);
        }
    }
}
typedef struct {
     unsigned long time;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
} timerData;
zval fiber;

_Noreturn int thr(void *loop) {
    printf("thred started\n");
    uv_timer_t timerHandle;
    uv_timer_init(FILE_IO_GLOBAL(loop), &timerHandle);
    timerData * td= (timerData *)loop;

    fill_handle_with_data(&timerHandle, &td->fci, &td->fcc);
    printf("time is in thrd prc %lu  %p\n", td->time, &td->time);
    uv_timer_start(&timerHandle, fn, td->time, 0);
    printf("\nloop %d, p:=%p\n", uv_loop_alive(FILE_IO_GLOBAL(loop)), FILE_IO_GLOBAL(loop));
    printf("Active = %d\n", FILE_IO_GLOBAL(loop)->active_handles);
    printf("\nloop run %d\n", uv_run(FILE_IO_GLOBAL(loop), UV_RUN_DEFAULT));
    printf("\n after run loop %d, p:=%p\n", uv_loop_alive(FILE_IO_GLOBAL(loop)), FILE_IO_GLOBAL(loop));
    while (true);
//     uv_async_send(&as_h);
//     while (true){
//         printf("\nloop %d, p:=%p\n", uv_loop_alive((uv_loop_t*)loop), loop);
//         printf("Active = %d\n", ((uv_loop_t*)loop)->active_handles);
//
//         printf("\nloop run %d\n", uv_run((uv_loop_t *) (loop), UV_RUN_DEFAULT));
////
////         uv_run((uv_loop_t*)loop, UV_RUN_DEFAULT);
//         printf("thred finshed");
//         usleep(20);
//     }



}
static timerData timerData1={};
/* {{{ string test2( [ string $var ] ) */

PHP_FUNCTION (setTimeout) {
     zend_long var;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    zval * callable;
    zval return_val;
    zval retval;
//    object_init_ex(&fiber, zend_ce_fiber);
    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_FUNC(fci, fcc)
            Z_PARAM_LONG(var)ZEND_PARSE_PARAMETERS_END();
    fci.retval = &return_val;
    fci.param_count = 0;
    timerData1.time = var;
    memcpy(&timerData1.fci, &fci, sizeof(zend_fcall_info));
    memcpy(&timerData1.fcc, &fcc, sizeof(zend_fcall_info_cache));

    printf("time is in main prc %lu  %p\n", timerData1.time, &timerData1.time);
//    zend_call_known_instance_method_with_1_params(Z_OBJCE(fiber)->constructor, Z_OBJ(fiber), NULL, callable);
//    zend_call_method_with_0_params(Z_OBJ(fiber), Z_OBJCE(fiber), NULL, "start", NULL);
    thrd_create(&thrd, thr, &timerData1);
    printf("test\n");
//
//    printf("P = %p", fileio_globals.loop);

   printf( " left %d\n", 1);

//    thrd_join(thrd, NULL);

    RETURN_NULL();
}
/* }}}*/

PHP_FUNCTION (retbool) {
    RETURN_BOOL(INI_BOOL("file_io.use_promise"));
}

PHP_INI_BEGIN()
                PHP_INI_ENTRY("file_io.use_promise", 0, PHP_INI_ALL, NULL)
PHP_INI_END()


/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION (fileio) {

    fileio_globals.loop = uv_default_loop();
//    uv_async_init(fileio_globals.loop, &as_h, NULL);
//    uv_loop_fork(fileio_globals.loop);

    REGISTER_INI_ENTRIES();
#if defined(ZTS) && defined(COMPILE_DL_FILEIO)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
//TODO: add init!!

    return SUCCESS;
}

/* {{{ PHP_MINIT_FUNCTION */
PHP_RSHUTDOWN_FUNCTION (fileio) {
//    printf("Call back is ended");
//   thrd_join(thrd, NULL);
//   uv_loop_close(FILE_IO_GLOBAL(loop));
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION (fileio) {
    UNREGISTER_INI_ENTRIES();
//    uv_loop_close(FILE_IO_GLOBAL(loop));
//    free(FILE_IO_GLOBAL(loop));
}

/* }}} */
/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION (fileio) {
#if defined(ZTS) && defined(COMPILE_DL_FILEIO)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION (fileio) {
    php_info_print_table_start();
    php_info_print_table_header(2, "fileio support", "enabled");
    php_info_print_table_end();
}
/* }}} */

/* {{{ fileio_module_entry */
zend_module_entry fileio_module_entry = {
        STANDARD_MODULE_HEADER,
        "fileio",                    /* Extension name */
        file_io_functions,                    /* zend_function_entry */
        PHP_MINIT(fileio),                            /* PHP_MINIT - Module initialization */
        PHP_MSHUTDOWN(fileio),                            /* PHP_MSHUTDOWN - Module shutdown */
        PHP_RINIT(fileio),            /* PHP_RINIT - Request initialization */
        NULL,                            /* PHP_RSHUTDOWN - Request shutdown */
        PHP_MINFO(fileio),            /* PHP_MINFO - Module info */
        PHP_FILEIO_VERSION,        /* Version */
        STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_FILEIO
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(fileio)
#endif
