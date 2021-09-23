//
// Created by admin on 14.09.2021.
//

#ifndef FILEIO_CALLBACK_INTERFACE_H
#define FILEIO_CALLBACK_INTERFACE_H
#include <uv.h>
typedef struct {
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
} uv_cb_type;
#endif //FILEIO_CALLBACK_INTERFACE_H
void fn(uv_timer_t *handle);
void fn_idle(uv_idle_t *handle);
void fn_interval(uv_timer_t *handle);
