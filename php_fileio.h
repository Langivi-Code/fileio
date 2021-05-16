/* fileio extension for PHP */

#ifndef PHP_FILEIO_H
# define PHP_FILEIO_H

#include <uv.h>
#include <zend_modules.h>


typedef struct _zend_fileio_globals { //ZEND_DECLARE_MODULE_GLOBALS(fileio)
    uv_loop_t * loop;
} zend_fileio_globals; // ZEND_EXTERN_MODULE_GLOBALS(fileio)

zend_fileio_globals fileio_globals = {NULL}; //ZEND_DECLARE_MODULE_GLOBALS(fileio);

extern zend_module_entry fileio_module_entry;
# define phpext_fileio_ptr &fileio_module_entry

# define PHP_FILEIO_VERSION "0.1.0"

typedef void  (*cbf)(void *, char *);

void async_read(char *, cbf);

# if defined(ZTS) && defined(COMPILE_DL_FILEIO)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif    /* PHP_FILEIO_H */
