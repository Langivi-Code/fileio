#include <php.h>
#include <zend_API.h>
#include <uv.h>
#include "../common/fill_event_handle.h"
#include "../common/callback_interface.h"
#include "../../php_fileio.h"

#include "../../fileio_arginfo.h"
#include "idle_interface.h"


void fill_idle_handle_with_data(
        uv_idle_t *idle_type,
        zend_fcall_info *fci,
        zend_fcall_info_cache *fcc
) {
    uv_cb_type uv = {};
    printf("size of idle handler %lu, fci  %lu \n\n", sizeof *idle_type, sizeof *fci);
    idle_type->data = (uv_cb_type *) emalloc(sizeof(uv_cb_type));
    fill_event_handle(idle_type, fci, fcc, &uv);
}


PHP_FUNCTION (idle) {
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    zval return_val;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_FUNC(fci, fcc)ZEND_PARSE_PARAMETERS_END();
    fci.retval = &return_val;
    fci.param_count = 0;
    uv_idle_t *idleHandle = emalloc(sizeof(uv_idle_t));

    uv_idle_init(FILE_IO_GLOBAL(loop), idleHandle);
    fill_idle_handle_with_data(idleHandle, &fci, &fcc);
    printf("Setting idle ...\n");
    uv_idle_start(idleHandle, fn_idle);

    RETURN_NULL();
}
/* }}}*/