//
// Created by admin on 28.09.2021.
//

#ifndef FILEIO_FILE_INTERFACE_H
#define FILEIO_FILE_INTERFACE_H

#include <uv.h>
#include <zend_API.h>
#include "../../constants.h"
#include "../common/callback_interface.h"
#include "../common/struct.h"
typedef struct {
    uv_fs_t * open_req;
    uv_fs_t * read_req;
    uv_fs_t * write_req;
    uv_fs_t * status_req;
} fs_close_reqs_t;
typedef struct {
    unsigned long long id
} fs_id_t;
typedef struct  {
    uv_cb_type php_cb_data;
    uv_file file;
    char * filename;
    uint64_t file_size;
    uv_buf_t buffer;
    bool read;
    fs_close_reqs_t close_requests;
    unsigned long long handle_id;
} file_handle_data;

typedef struct {
    unsigned long long handle_id;
    uv_fs_t * open_req;
} fs_handles_id_item_t;

ADD_STRUCT(fs, file_handle_data);

extern fs_id_item_t fs_handle_map[HANDLE_MAP_SIZE];
CREATE_HANDLE_LIST_HEADERS(fs, file_handle_data);

void close_cb(uv_fs_t* req);
#endif //FILEIO_FILE_INTERFACE_H