#include <php.h>
#include <zend_API.h>
#include <uv.h>
#include "../../constants.h"
#include "../../functions/common/callback_interface.h"
#include "../../php_fileio.h"
#include "../../fileio_arginfo.h"
#include "timers_interface.h"
#include <string.h>




handle_id_item_t interval_handle_map[HANDLE_MAP_SIZE];

static unsigned short count_handles() {
    unsigned short i = 0;
    for (; i < HANDLE_MAP_SIZE; i++) {
        if (interval_handle_map[i].handle_id == 0) {
            printf(" i %d handle_Id  %llu\n", i, interval_handle_map[i].handle_id);
            break;
        }
    }
    return i;
}

static unsigned long long add_handle(uv_timer_t *handle) {
    unsigned short handle_count = count_handles();
    interval_handle_map[handle_count] = (handle_id_item_t) {uv_now(FILE_IO_GLOBAL(loop)), handle};
    return interval_handle_map[handle_count].handle_id;
}

static handle_id_item_t * find_handle(unsigned long long handleId) {
    unsigned short i = 0;
    for (; i < HANDLE_MAP_SIZE; i++) {
        printf(" searching %d  handle_Id  %llu\n", i, interval_handle_map[i].handle_id, handleId);
        if (interval_handle_map[i].handle_id == handleId) {
            printf(" i %d found handle_Id  %llu\n", i, interval_handle_map[i].handle_id);
            return &interval_handle_map[i];
        }
    }
}

static void remove_handle(unsigned long long handleId) {
    handle_id_item_t *tempItems = malloc(1024 * sizeof(handle_id_item_t));
    unsigned short i = 0;
    unsigned short tagret = 0;
    for (; i < HANDLE_MAP_SIZE; i++) {
        if (interval_handle_map[i].handle_id == handleId) {
            printf(" i %d  removed handle_Id  %llu\n", i, interval_handle_map[i].handle_id);
            continue;
        }
        tempItems[tagret] = interval_handle_map[i];
        tagret++;
    }
    memcpy(interval_handle_map, tempItems, 1024 * sizeof(handle_id_item_t));
    free(tempItems);
}

PHP_FUNCTION (setInterval) {
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
    fill_timer_handle_with_data(timerHandle, &fci, &fcc);
    printf("time is in thrd prc %lld  %p\n", var, &var);
    unsigned long id = add_handle(timerHandle);
    uv_timer_start(timerHandle, fn_interval, var, var);

    printf("handle id %lul handles count is %ul\n", id, count_handles());
    //remove_handle(id);
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

    RETURN_LONG(id);
}
/* }}}*/

PHP_FUNCTION (clearInterval) {
    zend_long timer_id;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_LONG(timer_id)ZEND_PARSE_PARAMETERS_END();

    printf("\nclearing timeout %lu\n",timer_id);
    if (timer_id > 0) {
        handle_id_item_t *timer_handle = find_handle(timer_id);
        printf("$id = %llu", timer_handle->handle_id);
        uv_timer_stop((uv_timer_t *) timer_handle->handle);
        efree(timer_handle->handle);
        printf("handle id %lul handles count is %ul\n", timer_id, count_handles());
        remove_handle(timer_id);
        printf("handle id %lul handles count is %ul\n", timer_id, count_handles());
    } else {
        RETURN_FALSE;
    }
    RETURN_TRUE;

}