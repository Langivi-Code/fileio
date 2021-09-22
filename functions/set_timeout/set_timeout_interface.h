#include "../../constants.h"

//
// Created by admin on 14.09.2021.
//
void fill_timeout_handle_with_data(
        uv_timer_t *handle,
        zend_fcall_info *fci,
        zend_fcall_info_cache *fcc
);
#ifndef FILEIO_SET_TIMEOUT_INTERFACE_H
#define FILEIO_SET_TIMEOUT_INTERFACE_H
typedef struct {
    unsigned long time;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
} timerData;

typedef struct {
    unsigned long handle_id;
    void * handle
} handle_id_item_t;
extern handle_id_item_t handle_map[HANDLE_MAP_SIZE];
#endif //FILEIO_SET_TIMEOUT_INTERFACE_H
