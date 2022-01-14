/* file io extension for PHP */

#ifndef PHP_FILEIO_H
# define PHP_FILEIO_H
# define PHP_FILEIO_VERSION "1.0.1"
#include <uv.h>
#include <zend_modules.h>
#ifdef ZTS
#include "TSRM.h"
#endif

//typedef struct _zend_fileio_globals {
ZEND_BEGIN_MODULE_GLOBALS(standard_async)
    uv_loop_t *loop;
    zend_class_entry *promise_class;
    zend_class_entry *mysqli_async_class;
    zend_class_entry *server_class;
    zend_class_entry *http_server_class;
    zend_class_entry *http_request_class;
    zend_class_entry *http_response_class;
    zend_class_entry *promise__status_enum;
    zend_class_entry *async_fs_exception;
//} zend_fileio_globals;
ZEND_END_MODULE_GLOBALS(standard_async)

//zend_fileio_globals fileio_globals = {NULL};
//extern zend_fileio_globals v8js_globals;

ZEND_EXTERN_MODULE_GLOBALS(standard_async)

extern zend_module_entry fileio_module_entry;
#define phpext_fileio_ptr &fileio_module_entry



#ifdef ZTS
#define MODULE_GL(v) TSRMG(fileio_globals_id, zend_fileio_globals *, v)
#else
#define MODULE_GL(v) standard_async_globals.v
#endif




# if defined(ZTS) && defined(COMPILE_DL_FILEIO)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif    /* PHP_FILEIO_H */
