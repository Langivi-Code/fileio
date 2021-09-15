#include <php.h>
#include <zend_API.h>
#include <uv.h>
#include "../common/callback_interface.h"
#include "../../php_fileio.h"
//#include <threads.h>
#include "../../fileio_arginfo.h"
#include "set_timeout_interface.h"

static timerData timerData1 = {};
extern void fill_handle_with_data(
        uv_timer_t *handle,
        zend_fcall_info *fci,
        zend_fcall_info_cache *fcc
);



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
    uv_timer_t *  timerHandle = malloc(sizeof(uv_timer_t));


    uv_timer_init(FILE_IO_GLOBAL(loop), timerHandle);
    fill_handle_with_data(timerHandle, &fci, &fcc);
    printf("time is in thrd prc %lu  %p\n", var, &var);
    uv_timer_start(timerHandle, fn, var, 0);
//    memcpy(&timerData1.fci, &fci, sizeof(zend_fcall_info));
//    memcpy(&timerData1.fcc, &fcc, sizeof(zend_fcall_info_cache));

//    printf("time is in main prc %lu  %p\n", timerData1.time, &timerData1.time);
    //    zend_call_known_instance_method_with_1_params(Z_OBJCE(fiber)->constructor, Z_OBJ(fiber), NULL, callable);
    //    zend_call_method_with_0_params(Z_OBJ(fiber), Z_OBJCE(fiber), NULL, "start", NULL);
//    thrd_create(&thrd, thr, &timerData1);
    printf("test\n");
    //
    //    printf("P = %p", fileio_globals.loop);

    printf( " left %d\n", 1);

    //    thrd_join(thrd, NULL);

    RETURN_NULL();
}
/* }}}*/