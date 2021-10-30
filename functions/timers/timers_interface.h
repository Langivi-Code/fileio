#include "../../constants.h"

//
// Created by admin on 14.09.2021.
//
#include <uv.h>
#include <php.h>
#include <zend_API.h>
#include "../../php_fileio.h"
#ifndef FILEIO_SET_TIMEOUT_INTERFACE_H
#define FILEIO_SET_TIMEOUT_INTERFACE_H
typedef struct {
    unsigned long time;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
} timerData;

typedef struct {
    unsigned long long handle_id;
    uv_timer_t * handle;
} handle_id_item_t;
void fill_timer_handle_with_data(
        uv_timer_t *handle,
        zend_fcall_info *fci,
        zend_fcall_info_cache *fcc
);
extern handle_id_item_t timer_handle_map[HANDLE_MAP_SIZE];
unsigned short count_handles();
unsigned long long add_handle(uv_timer_t *handle);
handle_id_item_t * find_handle(unsigned long long handleId);
void remove_handle(unsigned long long handleId);

#endif //FILEIO_SET_TIMEOUT_INTERFACE_H
