//
// Created by admin on 28.09.2021.
//
#include "file_interface.h"
#include "../common/fill_event_handle.h"
#include <php.h>
#include <zend_API.h>
#define LOG_TAG "Fill FS request data"
void fill_fs_handle_with_data(
        uv_fs_t *handle,
        file_handle_data * handleData
) {
    LOG("uv_fs_t struct(%lubytes), file_handle_data struct(%lubytes)", sizeof *handle, sizeof *handleData);
    handle->data = handleData;
}

