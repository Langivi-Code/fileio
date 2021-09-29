//
// Created by admin on 28.09.2021.
//

#ifndef FILEIO_FILE_INTERFACE_H
#define FILEIO_FILE_INTERFACE_H

#endif //FILEIO_FILE_INTERFACE_H
#include <uv.h>
#include <zend_API.h>
#include "../common/callback_interface.h"

typedef struct  {
    uv_cb_type php_cb_data;
    uv_file file;
    char * filename;
    uint64_t file_size;
    uv_buf_t buffer;
    unsigned long long handle_id
} file_handle_data;

typedef struct {
    unsigned long long handle_id;
    uv_fs_t * open_req;
} fs_handles_id_item_t;
void fill_fs_handle_with_data(
        uv_fs_t *handle,
        file_handle_data * handleData
);