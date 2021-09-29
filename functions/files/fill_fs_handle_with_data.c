//
// Created by admin on 28.09.2021.
//
#include "file_interface.h"
#include "../common/fill_event_handle.h"
#include <php.h>
#include <zend_API.h>

void fill_fs_handle_with_data(file_handle_data * handleData
        uv_fs_t *handle,
        zend_fcall_info *fci,
        zend_fcall_info_cache *fcc
) {
    uv_cb_type uv = {};
    printf("size of timeout handler %lu, fci  %lu \n\n", sizeof *handle, sizeof *fci);
    handle->data = (uv_cb_type *) emalloc(sizeof(uv_cb_type));
    fill_event_handle(handle, fci, fcc, &uv);
}