//
// Created by user on 01.10.21.
//

#include "file_interface.h"
#include "../../constants.h"
#include <uv.h>
#define LOG_TAG "file_handles"
__attribute__((unused)) fs_handles_id_item_t fstimeout_handle_map[HANDLE_MAP_SIZE];

unsigned short count_fs_handles() {
    unsigned short i = 0;
    for (; i < HANDLE_MAP_SIZE; i++) {
        if (fstimeout_handle_map[i].handle_id == 0) {
            LOG("Handles count - %d, last handle_id is %llu", i, fstimeout_handle_map[i].handle_id);
            break;
        }
    }
    return i;
}

unsigned long long add_fs_handle(uv_fs_t *handle) {
    unsigned short handle_count = count_fs_handles();
    fstimeout_handle_map[handle_count] = (fs_handles_id_item_t) {uv_hrtime(), handle};

    return fstimeout_handle_map[handle_count].handle_id;
}

fs_handles_id_item_t *find_fs_handle(unsigned long long handleId) {
    unsigned short i = 0;
    for (; i < HANDLE_MAP_SIZE; i++) {
        if (fstimeout_handle_map[i].handle_id == handleId) {
            LOG("Searching element #%d with handle_id=%llu", i, fstimeout_handle_map[i].handle_id);
            return &fstimeout_handle_map[i];
        }
    }
}

void remove_fs_handle(unsigned long long handleId) {
    fs_handles_id_item_t *tempItems = malloc(HANDLE_MAP_SIZE * sizeof(fs_handles_id_item_t));
    unsigned short i = 0;
    unsigned short tagret = 0;
    for (; i < HANDLE_MAP_SIZE; i++) {
        if (fstimeout_handle_map[i].handle_id == handleId) {
            LOG(" element #%d with handle_id=%llu was removed", i, fstimeout_handle_map[i].handle_id);
            continue;
        }
        tempItems[tagret] = fstimeout_handle_map[i];
        tagret++;
    }
    memcpy(fstimeout_handle_map, tempItems, HANDLE_MAP_SIZE * sizeof(fs_handles_id_item_t));
    free(tempItems);
}

void close_cb(uv_fs_t* req) {
    int result = req->result;

    if (result == -1) {
        fprintf(stderr, "Error closing file: %s.\n",
                uv_strerror(result));
    }
    fs_close_reqs_t *handle = (fs_close_reqs_t *) req->data;

    uv_fs_req_cleanup(req);
    if(handle->write_req != NULL){
        file_handle_data *data_handle = (file_handle_data *) handle->write_req->data;
        remove_fs_handle(data_handle->handle_id);
        efree(data_handle);
        uv_fs_req_cleanup(handle->write_req);
        efree(handle->write_req);
    }
    if(handle->open_req != NULL){
        uv_fs_req_cleanup(handle->open_req);
        efree(handle->open_req);
    }

    if(handle->read_req != NULL){
        file_handle_data *data_handle = (file_handle_data *) handle->read_req->data;
        remove_fs_handle(data_handle->handle_id);
        efree(data_handle);
        uv_fs_req_cleanup(handle->read_req);
        efree(handle->read_req);
    }
    printf("Close CB: Successfuly closed file.\n");
}

void fill_file_handle(file_handle_data *handleData, char *filename,
                      zend_fcall_info *fci,
                      zend_fcall_info_cache *fcc) {
    handleData->filename = filename;
    memcpy(&handleData->php_cb_data.fci, fci, sizeof(zend_fcall_info));
    memcpy(&handleData->php_cb_data.fcc, fcc, sizeof(zend_fcall_info_cache));
}