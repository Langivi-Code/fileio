/* fileio extension for PHP */

#ifdef HAVE_CONFIG_H

# include "config.h"

#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include <TSRM.h>
#include "php_fileio.h"
#include "zend_API.h"

#include "zend_closures.h"
#include "zend_interfaces.h"
#include "fileio_arginfo.h"
#include <stdio.h>
#include <wchar.h>
#include <fcntl.h>
#include "uv.h"
#include "functions/timers/timers_interface.h"
#include "functions/common/callback_interface.h"
#include "constants.h"
#include <ext/standard/basic_functions.h>
#include <SAPI.h>
#include <zend_smart_str.h>
#include <json/php_json.h>
#include "./functions/web/helpers.h"
#include "./functions/web/server.h"
#include "./functions/files/file_interface.h"

extern zend_class_entry *create_PromiseStatus_enum(void);

extern zend_class_entry *register_class_Promise(void);
extern zend_class_entry *register_class_Server(void);
extern zend_class_entry *register_class_HttpServer(void);
extern zend_class_entry *register_class_HttpRequest(void);
extern zend_class_entry *register_class_HttpResponse(void);
extern  zend_function * promise_resolve;
extern  zend_function * promise_reject;
ZEND_DECLARE_MODULE_GLOBALS(fileio);


//extern PHP_FIBER_API zend_class_entry *zend_ce_fiber;
/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
    ZEND_PARSE_PARAMETERS_START(0, 0) \
    ZEND_PARSE_PARAMETERS_END()
#endif


/* }}} */


void print(uv_async_t *handle) {
    printf(" print thread id: %ld, value is %ld\n", uv_thread_self(), (long) handle->data);

}

//void after(uv_work_t *req, int status) {
//    printf("done, thread id: %ld\n", uv_thread_self());
//
//}
//
//void run(uv_work_t *req) {
//    long count = (long) req->data;
//
//    for (int index = 0; index < count; index++) {
//        printf("run thread id: %ld, index: %d\n", uv_thread_self(), index);
//        async.data = (void *) (long) index;
////        uv_async_send(&async);
//        sleep(1);
//
//    }
//
//}

//_Noreturn int thr(void *loop) {
//    printf("thred started\n");
//    uv_timer_t timerHandle;
//
//    timerData *td = (timerData *) loop;
//
//    fill_timeout_handle_with_data(&timerHandle.data, &td->fci, &td->fcc);
//    printf("time is in thrd prc %lu  %p\n", td->time, &td->time);
//    uv_timer_start(&timerHandle, fn, td->time, 0);
//    printf("\nloop %d, p:=%p\n", uv_loop_alive(FILE_IO_GLOBAL(loop)), FILE_IO_GLOBAL(loop));
//    printf("Active = %d\n", FILE_IO_GLOBAL(loop)->active_handles);
//    printf("\nloop run %d\n", uv_run(FILE_IO_GLOBAL(loop), UV_RUN_DEFAULT));
//    printf("\n after run loop %d, p:=%p\n", uv_loop_alive(FILE_IO_GLOBAL(loop)), FILE_IO_GLOBAL(loop));
//    while (true);
////     uv_async_send(&as_h);
////     while (true){
////         printf("\nloop %d, p:=%p\n", uv_loop_alive((uv_loop_t*)loop), loop);
////         printf("Active = %d\n", ((uv_loop_t*)loop)->active_handles);
////
////         printf("\nloop run %d\n", uv_run((uv_loop_t *) (loop), UV_RUN_DEFAULT));
//////
//////         uv_run((uv_loop_t*)loop, UV_RUN_DEFAULT);
////         printf("thred finshed");
////         usleep(20);
////     }
//
//
//
//}

/* {{{ string test2( [ string $var ] ) */

ZEND_FUNCTION(enable_event) {
#define LOG_TAG "enable_event"
    LOG("starting event loop %c", 50);
    uv_loop_t * loop = FILE_IO_GLOBAL(loop);
    LOG("size of ev-queue %d(Active = %d), loop address:=%p", uv_loop_alive(loop), loop->active_handles, loop);
    printf("loop run status: %d\n", uv_run(loop, UV_RUN_DEFAULT));
    LOG("size of ev-queue after run %d (Active = %d), loop address:=%p", uv_loop_alive(loop), loop->active_handles, loop);
    uv_loop_close(FILE_IO_GLOBAL(loop));
//    sleep(10);
//    uv_async_init(fileio_globals.loop, &as_h, NULL);
//    uv_loop_fork(fileio_globals.loop);
}


/**********************************************/
/***        MODULE SETTINGS SECTION         ***/
/**********************************************/

PHP_INI_BEGIN()
        PHP_INI_ENTRY("file_io.use_promise", 0, PHP_INI_ALL, NULL)
PHP_INI_END()
SAPI_API SAPI_POST_HANDLER_FUNC(json_post_handler)
{
    zval *arr = (zval *) arg;
    php_stream *s = SG(request_info).request_body;
    smart_str buff={0};

    if (s && SUCCESS == php_stream_rewind(s)) {
        while (!php_stream_eof(s)) {
            char buf[BUFSIZ] = {0};
            ssize_t len = php_stream_read(s, buf, BUFSIZ);

            if (len > 0) {
                smart_str_appendl(&buff, buf, len);
            }

            if (len != BUFSIZ){
                break;
            }
        }
        zval ret;
        zend_long options = 0;
        options |=  PHP_JSON_OBJECT_AS_ARRAY;
        if (buff.s) {
            if(php_json_decode_ex(&ret, ZSTR_VAL(buff.s), buff.a, options, 512)==SUCCESS){
                fill_super_global(TRACK_VARS_POST, &ret);
            }
        }
    }
}
/**********************************************/
/***        MODULE INTIALIZATION SECTION    ***/
/**********************************************/
#define JSON_POST_CONTENT_TYPE "application/json"
/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION (fileio) {
    FILE_IO_GLOBAL(loop) = uv_default_loop();
    create_PromiseStatus_enum();
    register_class_Promise();
    register_class_HttpServer();
    register_class_HttpRequest();
    register_class_HttpResponse();
    register_class_Server();
    REGISTER_INI_ENTRIES();
    promise_resolve = zend_hash_str_find_ptr(&FILE_IO_GLOBAL(promise_class->function_table), "resolve", sizeof("resolve")-1);
    promise_reject = zend_hash_str_find_ptr(&FILE_IO_GLOBAL(promise_class->function_table), "reject", sizeof("reject")-1);
    static const sapi_post_entry php_post_entries = { JSON_POST_CONTENT_TYPE,    sizeof(JSON_POST_CONTENT_TYPE)-1,    NULL, json_post_handler };
    sapi_register_post_entry(&php_post_entries);

#if defined(ZTS) && defined(COMPILE_DL_FILEIO)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    return SUCCESS;
}

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION (fileio) {
//    PG(auto_prepend_file)="Promise.php";
    memset(timer_handle_map,0, HANDLE_MAP_SIZE * sizeof(handle_id_item_t));
    memset(fstimeout_handle_map,0, HANDLE_MAP_SIZE * sizeof(fs_handles_id_item_t));
    memset(php_servers, 0, sizeof(server_type) * 10);
    memset(http_php_servers, 0, sizeof(http_server_type) * 10);
#if defined(ZTS) && defined(COMPILE_DL_FILEIO)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    zend_function * func = zend_hash_str_find_ptr(EG(function_table), "enable_event", strlen("enable_event"));
    php_shutdown_function_entry shutdown_function_entry = {};
    zval callable;
    zval callable2;
    zend_result result;

    ZVAL_FUNC(&callable2, func);
//    ZVAL_STRING(&callable2,"var_dump");
    ZVAL_STRINGL(&callable, "enable_event", strlen("enable_event"));
//
////    zval params[1];
////    ZVAL_COPY_VALUE(&params[0], &callable);
//
    result = zend_fcall_info_init(&callable, 0, &shutdown_function_entry.fci,
                                  &shutdown_function_entry.fci_cache, NULL, NULL);
    printf("%d\n", result);
    shutdown_function_entry.fci_cache.function_handler = func;
    shutdown_function_entry.fci.param_count=0;
////    shutdown_function_entry.fci.params=&params;
    append_user_shutdown_function(&shutdown_function_entry);
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
//    free(FILE_IO_GLOBAL(loop));
    return SUCCESS;
}

/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION (fileio) {
    php_info_print_table_start();
    php_info_print_table_header(2, "fileio support", "enabled");
    php_info_print_table_row(2, "timers", "enabled");
    php_info_print_table_row(2, "idle work", "enabled");
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
ZEND_GET_MODULE(fileio);

#endif
