//
// Created by admin on 28.09.2021.
//

#ifndef FILEIO_FILE_INTERFACE_H
#define FILEIO_FILE_INTERFACE_H

#include <uv.h>
#include <zend_API.h>
#include "../../constants.h"
#include "../common/callback_interface.h"

typedef struct  {
    uv_cb_type php_cb_data;
    uv_file file;
    char * filename;
    uint64_t file_size;
    uv_buf_t buffer;
    uv_fs_t * open_req;
    bool read;
    unsigned long long handle_id;
} file_handle_data;

typedef struct {
    unsigned long long handle_id;
    uv_fs_t * open_req;
} fs_handles_id_item_t;

typedef struct {
    uv_fs_t * open_req;
    uv_fs_t * read_req;
    uv_fs_t * write_req;
} fs_close_reqs_t;

__attribute__((unused)) extern fs_handles_id_item_t fstimeout_handle_map[HANDLE_MAP_SIZE];

void fill_fs_handle_with_data(
        uv_fs_t *handle,
        file_handle_data * handleData
);
unsigned short count_fs_handles();
unsigned long long add_fs_handle(uv_fs_t *handle);
fs_handles_id_item_t *find_fs_handle(unsigned long long handleId);
void remove_fs_handle(unsigned long long handleId);
void close_cb(uv_fs_t* req);
void fill_file_handle(file_handle_data *handleData, char *filename,
                      zend_fcall_info *fci,
                      zend_fcall_info_cache *fcc);
#endif //FILEIO_FILE_INTERFACE_H