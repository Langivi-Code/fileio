#include <php.h>
#include <zend_API.h>
#include <uv.h>
#include "../common/fill_event_handle.h"
#include "../common/callback_interface.h"
#include "../../php_fileio.h"
#include "../../constants.h"



#include "../../fileio_arginfo.h"
#include "idle_interface.h"

#define LOG_TAG "idle"

PHP_FUNCTION (idle) {
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    zval return_val;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_FUNC(fci, fcc)ZEND_PARSE_PARAMETERS_END();
    fci.retval = &return_val;
    fci.param_count = 0;
    uv_idle_t *idleHandle = emalloc(sizeof(uv_idle_t));

    uv_idle_init(MODULE_GL(loop), idleHandle);
    fill_event_handle(idleHandle, &fci, &fcc);
    LOG("Setting idle ...\n");
    uv_idle_start(idleHandle, fn_idle);

    RETURN_NULL();
}

/* }}}*/