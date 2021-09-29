//
// Created by admin on 28.09.2021.
//

#ifndef FILEIO_FILE_INTERFACE_H
#define FILEIO_FILE_INTERFACE_H

#endif //FILEIO_FILE_INTERFACE_H
#include <uv.h>
#include <zend_API.h>
typedef struct  {
    uv_cb_type php_cb_data;
    uv_file file;
    char * filename;
    size_t file_size;
    uv_buf_t buffer;
} file_handle_data;

void fill_fs_handle_with_data(file_handle_data * handleData,
        uv_fs_t *handle,
        zend_fcall_info *fci,
        zend_fcall_info_cache *fcc
);