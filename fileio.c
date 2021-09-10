/* fileio extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/file.h"
#include "php_fileio.h"
#include "Zend_API.h"
#include "fileio_arginfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <wchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
    ZEND_PARSE_PARAMETERS_START(0, 0) \
    ZEND_PARSE_PARAMETERS_END()
#endif

/* {{{ void file_get_contents_async() */
PHP_FUNCTION (file_get_contents_async) {
    char *filename;
    size_t filename_len;
    zend_bool use_include_path = 0;
    php_stream *stream;
    zend_long offset = 0;
    zend_long maxlen;
    zend_bool maxlen_is_null = 1;
    zval * zcontext = NULL;
    php_stream_context *context = NULL;
    zend_string * contents;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    zval retval;            /* Function return value */
    zval arg;                /* Argument to pass to function */
    /* Parse arguments */
    ZEND_PARSE_PARAMETERS_START(1, 6)
            Z_PARAM_PATH(filename, filename_len)
            Z_PARAM_FUNC(fci, fcc)
            Z_PARAM_OPTIONAL
            Z_PARAM_BOOL(use_include_path)
            Z_PARAM_RESOURCE_OR_NULL(zcontext)
            Z_PARAM_LONG(offset)
            Z_PARAM_LONG_OR_NULL(maxlen, maxlen_is_null)ZEND_PARSE_PARAMETERS_END();

    if (maxlen_is_null) {
        maxlen = (ssize_t) PHP_STREAM_COPY_ALL;
    } else if (maxlen < 0) {
        zend_argument_value_error(5, "must be greater than or equal to 0");
        RETURN_THROWS();
    }
    int error = uv_fs_open(fileio_globals.loop, (uv_fs_t *) &uv->uv.fs, filename, O_RDONLY, S_IRUSR, php_uv_fs_cb);
    if (error) {
        PHP_UV_DEINIT_UV(uv);
        php_error_docref(NULL, E_WARNING, "uv_open failed");
        return;
        fci.retval = &retval;
        fci.param_count = 1;
        fci.params = &arg;
        int ret;

        //OR
        ZVAL_COPY(&arg, zv);
        ret = zend_call_function(&fci, &fcc);
        i_zval_ptr_dtor(&arg);
        if (ret != SUCCESS || Z_TYPE(retval) == IS_UNDEF) {
            zend_array_destroy(Z_ARR_P(return_value));
            RETURN_NULL();
        }

        //OR
        if (zend_call_function(&fci, &fcc) == SUCCESS && Z_TYPE(retval) != IS_UNDEF) {
            if (EXPECTED(Z_TYPE(retval) == IS_STRING)) {
                contents = Z_STR(retval);
            } else {
                contents = zval_get_string_func(&retval);
                zval_ptr_dtor(&retval);
            }
        } else {
            if (!EG(exception)) {
                php_error_docref(NULL, E_WARNING, "Unable to call custom replacement function");
            }
        }
        context = php_stream_context_from_zval(zcontext, 0);

        stream = php_stream_open_wrapper_ex(filename, "rb",
                                            (use_include_path ? USE_PATH : 0) | REPORT_ERRORS,
                                            NULL, context);
        if (!stream) {
            RETURN_FALSE;
        }

        if (offset != 0 && php_stream_seek(stream, offset, ((offset > 0) ? SEEK_SET : SEEK_END)) < 0) {
            php_error_docref(NULL, E_WARNING, "Failed to seek to position " ZEND_LONG_FMT " in the stream", offset);
            php_stream_close(stream);
            RETURN_FALSE;
        }

        if ((contents = php_stream_copy_to_mem(stream, maxlen, 0)) != NULL) {
            RETVAL_STR(contents);
        } else {
            RETVAL_EMPTY_STRING();
        }

        php_stream_close(stream);

        php_printf("The extension %s is loaded and working!\r\n", "fileio");
    }
}

/* }}} */
typedef struct {
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
} uv_cb_t;

void fn(uv_timer_t *handle) {
    zval retval = {0};
    uv_cb_t *uv = (uv_cb_t *) handle->data;
    zend_long error;
    if (ZEND_FCI_INITIALIZED(uv->fci)) {
        if (zend_call_function(&uv->fci, &uv->fcc) != SUCCESS) {
            error = -1;
        }
    } else {
        error = -2;
    }
}

void fn1(uv_timer_t *handle, zend_fcall_info fci,
         zend_fcall_info_cache fcc) {
    uv_cb_t *uv = (uv_cb_t *) handle->data;
    memcpy(&uv->fci, &fci, sizeof(zend_fcall_info));
    memcpy(&uv->fcc, &fcc, sizeof(zend_fcall_info_cache));

    if (ZEND_FCI_INITIALIZED(fci)) {
        Z_TRY_ADDREF(uv->fci.function_name);
        if (fci.object) {
            GC_ADDREF(fci.object);
        }
    }
}
/* {{{ string test2( [ string $var ] ) */
PHP_FUNCTION (setTimeout) {
    char *var1 = "World";
    size_t var1_len = sizeof("World") - 1;
    zend_long var;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_FUNC(fci, fcc)
            Z_PARAM_LONG(var)ZEND_PARSE_PARAMETERS_END();
    uv_timer_t timerHandle;
    fn1(&timerHandle,fci, fcc);
    uv_timer_init(FILE_IO_GLOBAL(loop), &timerHandle);

    uv_timer_start(&timerHandle, fn, var, 0);

    RETURN_NULL();
}
/* }}}*/

PHP_FUNCTION (exp) {
    RETURN_STRING(INI_BOOL("file_io.use_promise"));
}

PHP_INI_BEGIN()
                PHP_INI_ENTRY("file_io.use_promise", 0, PHP_INI_ALL, NULL)
PHP_INI_END()

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION (fileio) {

//    fileio_globals.loop = uv_default_loop();
    FILE_IO_GLOBAL(loop) = uv_default_loop();

    REGISTER_INI_ENTRIES();
#if defined(ZTS) && defined(COMPILE_DL_FILEIO)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
//TODO: add init!!

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION (fileio) {
    UNREGISTER_INI_ENTRIES();
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
