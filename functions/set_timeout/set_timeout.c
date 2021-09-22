#include <php.h>
#include <zend_API.h>
#include <uv.h>
#include "../../constants.h"
#include "../common/callback_interface.h"
#include "../common/fill_event_handle.h"
#include "../../php_fileio.h"
#include "../../fileio_arginfo.h"
#include "set_timeout_interface.h"
#include <string.h>
#include <limits.h>

void fill_timeout_handle_with_data(
        uv_timer_t *handle,
        zend_fcall_info *fci,
        zend_fcall_info_cache *fcc
) {
    uv_cb_type uv = {};
    printf("size of timeout handler %lu, fci  %lu \n\n", sizeof *handle, sizeof *fci);
    handle->data = (uv_cb_type *) emalloc(sizeof(uv_cb_type));
    fill_event_handle(handle, fci, fcc, &uv);
}


handle_id_item_t handle_map[HANDLE_MAP_SIZE];

unsigned short count_handles() {
    unsigned short i = 0;
    for (; i < HANDLE_MAP_SIZE; i++) {
        printf(" i %d handle_Id  %llu\n", i, handle_map[i].handle_id);
        if (handle_map[i].handle_id == 0) {
            break;
        }
    }
    return i;
}

unsigned long add_handle(void *handle) {
    unsigned short handle_count = count_handles();
    handle_map[handle_count] = (handle_id_item_t) {uv_now(FILE_IO_GLOBAL(loop)), handle};
    return handle_map[handle_count].handle_id;
}

handle_id_item_t * find_handle(unsigned short handleId) {
    unsigned short i = 0;
    for (; i < HANDLE_MAP_SIZE; i++) {
        printf(" i %d handle_Id  %llu\n", i, handle_map[i].handle_id);
        if (handle_map[i].handle_id == handleId) {
            return &handle_map[i];
        }
    }
}

void remove_handle(unsigned short handleId) {
    unsigned short handle_count = count_handles();
    handle_id_item_t *tempItems = malloc((handle_count - 1) * sizeof(handle_id_item_t));
    unsigned short i = 0;
    unsigned short tagret = 0;
    for (; i < HANDLE_MAP_SIZE; i++) {
        printf(" i %d handle_Id  %llu\n", i, handle_map[i].handle_id);
        if (handle_map[i].handle_id == handleId) {
            continue;
        }
        tempItems[tagret] = handle_map[i];
        tagret++;
    }
    memcpy(handle_map, tempItems, (handle_count - 1) * sizeof(handle_id_item_t));
}

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
    uv_timer_t *timerHandle = emalloc(sizeof(uv_timer_t));

    printf("Main thread id: %p\n", uv_thread_self());
    uv_timer_init(FILE_IO_GLOBAL(loop), timerHandle);
    fill_timeout_handle_with_data(timerHandle, &fci, &fcc);
    printf("time is in thrd prc %lld  %p\n", var, &var);
    unsigned long id = add_handle(timerHandle);
    uv_timer_start(timerHandle, fn, var, 0);

    printf("handle id %lul handles count is %ul\n", id, count_handles());
    remove_handle(id);
    printf("handle id %lul handles count is %ul\n", id, count_handles());
//    memcpy(&timerData1.fci, &fci, sizeof(zend_fcall_info));
//    memcpy(&timerData1.fcc, &fcc, sizeof(zend_fcall_info_cache));

//    printf("time is in main prc %lu  %p\n", timerData1.time, &timerData1.time);
    //    zend_call_known_instance_method_with_1_params(Z_OBJCE(fiber)->constructor, Z_OBJ(fiber), NULL, callable);
    //    zend_call_method_with_0_params(Z_OBJ(fiber), Z_OBJCE(fiber), NULL, "start", NULL);
//    thrd_create(&thrd, thr, &timerData1);
    //
    //    printf("P = %p", fileio_globals.loop);
    //    thrd_join(thrd, NULL);

    RETURN_NULL();
}
/* }}}*/

PHP_FUNCTION (clearTimeout) {
    zend_long timer_id;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_LONG(timer_id)ZEND_PARSE_PARAMETERS_END();

    if (timer_id > 0) {
        handle_id_item_t *timer_handle = find_handle(timer_id);
        uv_timer_stop((uv_timer_t *) timer_handle->handle);
        remove_handle(timer_id);
    } else {
        RETURN_FALSE;
    }
    RETURN_TRUE;

}