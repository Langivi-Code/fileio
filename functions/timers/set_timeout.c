#include <php.h>
#include <zend_API.h>
#include <uv.h>
#include "../../constants.h"
#include "../common/callback_interface.h"
#include "../../php_fileio.h"
#include "../../fileio_arginfo.h"
#include "timers_interface.h"
#include "../common/fill_event_handle.h"
#include <string.h>


PHP_FUNCTION (set_timeout) {
    zend_long var;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    zval return_val;
    //    object_init_ex(&fiber, zend_ce_fiber);
    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_FUNC(fci, fcc)
            Z_PARAM_LONG(var)ZEND_PARSE_PARAMETERS_END();
    fci.retval = &return_val;
    fci.param_count = 0;
    uv_timer_t *timerHandle = emalloc(sizeof(uv_timer_t));

//    printf("Main thread id: %p\n", uv_thread_self());
    uv_timer_init(MODULE_GL(loop), timerHandle);
    fill_event_handle(timerHandle, &fci, &fcc);
//    uv_unref((uv_handle_t *) timerHandle);
//    printf("time is in thrd prc %lld  %p\n", var, &var);
    unsigned long id = add_handle(timerHandle);
    uv_timer_start(timerHandle, fn, var, 0);

//    printf("handle id %lul handles count is %ul\n", id, count_handles());
    //remove_handle(id);
//    printf("handle id %lul handles count is %ul\n", id, count_handles());
//    memcpy(&timerData1.fci, &fci, sizeof(zend_fcall_info));
//    memcpy(&timerData1.fcc, &fcc, sizeof(zend_fcall_info_cache));

//    printf("time is in main prc %lu  %p\n", timerData1.time, &timerData1.time);
    //    zend_call_known_instance_method_with_1_params(Z_OBJCE(fiber)->constructor, Z_OBJ(fiber), NULL, callable);
    //    zend_call_method_with_0_params(Z_OBJ(fiber), Z_OBJCE(fiber), NULL, "start", NULL);
//    thrd_create(&thrd, thr, &timerData1);
    //
    //    printf("P = %p", fileio_globals.loop);
    //    thrd_join(thrd, NULL);

    RETURN_LONG(id);
}
/* }}}*/

PHP_FUNCTION (clear_timeout) {
    zend_long timer_id;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_LONG(timer_id)ZEND_PARSE_PARAMETERS_END();

    printf("\nclearing timeout %lu\n",timer_id);
    if (timer_id > 0) {
        handle_id_item_t *timer_handle = find_handle(timer_id);
        printf("$id = %llu", timer_handle->handle_id);
        uv_timer_stop((uv_timer_t *) timer_handle->handle);
        printf("handle id %lul handles count is %ul\n", timer_id, count_handles());
        remove_handle(timer_id);
        printf("handle id %lul handles count is %ul\n", timer_id, count_handles());
    } else {
        RETURN_FALSE;
    }
    RETURN_TRUE;

}