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
#include "set_timeout_interface.h"
#include "callback_interface.h"
//#include <threads.h>

extern void fn(uv_timer_t *handle);
//extern PHP_FIBER_API zend_class_entry *zend_ce_fiber;
/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
    ZEND_PARSE_PARAMETERS_START(0, 0) \
    ZEND_PARSE_PARAMETERS_END()
#endif



/* }}} */


//thrd_t thrd;
uv_async_t as_h;


void fill_handle_with_data(
        uv_timer_t *handle,
        zend_fcall_info *fci,
        zend_fcall_info_cache *fcc
) {
    uv_cb_type uv = {};
    printf("size %lu\n", sizeof handle->data);
    printf(" %lu \n", sizeof fci);
//    uv.fci = (zend_fcall_info *)emalloc(sizeof(zend_fcall_info));
handle->data = (uv_cb_type *) emalloc(sizeof(uv_cb_type));
    memcpy(&uv.fci, fci, sizeof(zend_fcall_info));
    memcpy(&uv.fcc, fcc, sizeof(zend_fcall_info_cache));
    memcpy(handle->data, &uv, sizeof(uv_cb_type));

    if (ZEND_FCI_INITIALIZED(*fci)) {
        Z_TRY_ADDREF(uv.fci.function_name);
        if (fci->object) {
            GC_ADDREF(fci->object);
        }
    }
}

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

/* {{{ string test2( [ string $var ] ) */





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

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION (fileio) {
#if defined(ZTS) && defined(COMPILE_DL_FILEIO)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION */
PHP_RSHUTDOWN_FUNCTION (fileio) {
//    printf("Call back is ended");
//   thrd_join(thrd, NULL);
//   uv_loop_close(FILE_IO_GLOBAL(loop));
    return SUCCESS;
}

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION (fileio) {
    UNREGISTER_INI_ENTRIES();
//    uv_loop_close(FILE_IO_GLOBAL(loop));
//    free(FILE_IO_GLOBAL(loop));
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
        "file_io",                    /* Extension name */
        file_io_functions,                    /* zend_function_entry */
        PHP_MINIT(fileio),                            /* PHP_MINIT - Module initialization */
        PHP_MSHUTDOWN(fileio),                            /* PHP_MSHUTDOWN - Module shutdown */
        PHP_RINIT(fileio),            /* PHP_RINIT - Request initialization */
        PHP_RSHUTDOWN(fileio),                            /* PHP_RSHUTDOWN - Request shutdown */
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
