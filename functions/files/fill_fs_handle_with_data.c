//
// Created by admin on 28.09.2021.
//
#include "file_interface.h"
#include "../common/fill_event_handle.h"
#include <php.h>
#include <zend_API.h>

void fill_fs_handle_with_data(
        uv_fs_t *handle,
        file_handle_data * handleData
) {
    uv_cb_type uv = {};
    printf("size of timeout handler %lu, fci  %lu \n\n", sizeof *handle, sizeof *handleData);
    handle->data = handleData;
}