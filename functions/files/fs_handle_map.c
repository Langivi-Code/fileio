//
// Created by user on 01.10.21.
//

#include "file_interface.h"
#include "../../constants.h"
#include <uv.h>
#define LOG_TAG "file_handles"
 fs_handles_id_item_t fstimeout_handle_map[HANDLE_MAP_SIZE];

unsigned short count_fs_handles() {
    unsigned short i = 0;
    for (; i < HANDLE_MAP_SIZE; i++) {
        if (fstimeout_handle_map[i].handle_id == 0) {
            LOG("Handles count - %d, last handle_id is %llu", i, fstimeout_handle_map[i - 1].handle_id);
            break;
        }
    }
    return i;
}

unsigned long long add_fs_handle(uv_fs_t *handle) {
    unsigned short handle_count = count_fs_handles();
    fstimeout_handle_map[handle_count] = (fs_handles_id_item_t) {uv_hrtime(), handle};

//    if(handle_count>0){
//        printf("ADD HANDLE %u\n",  memcmp( &fstimeout_handle_map[handle_count-1], &fstimeout_handle_map[handle_count],sizeof (fs_handles_id_item_t)));
//        printf("ADD FD HANDLE %u\n",  memcmp(&
//        (
//                (file_handle_data *)fstimeout_handle_map[handle_count-1].open_req->data
//                )->php_cb_data.fci, &
//        (
//                (file_handle_data *)fstimeout_handle_map[handle_count].open_req->data
//                )->php_cb_data.fci, sizeof (zend_fcall_info)));
//    }
    return fstimeout_handle_map[handle_count].handle_id;
}

fs_handles_id_item_t *find_fs_handle(unsigned long long handleId) {
    unsigned short i = 0;
    for (; i < HANDLE_MAP_SIZE; i++) {
        if (fstimeout_handle_map[i].handle_id == handleId) {
            LOG("Searching element #%d with handle_id=%llu", i, fstimeout_handle_map[i].handle_id);
            LOG("handle pointers %p %p", &fstimeout_handle_map[i].open_req, &fstimeout_handle_map[i]);
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

    if (handle->read_req != NULL) {
        file_handle_data *data_handle = (file_handle_data *) handle->read_req->data;
        remove_fs_handle(data_handle->handle_id);
        efree(data_handle);
        uv_fs_req_cleanup(handle->read_req);
        efree(handle->read_req);
    }
    printf("Close CB: Successfuly closed file.\n");
}
//pfci_dst = (zend_fcall_info *) safe_emalloc(1, sizeof(zend_fcall_info), 0);              \
//        pfcc_dst = (zend_fcall_info_cache *) safe_emalloc(1, sizeof(zend_fcall_info_cache), 0);  \
#define PHP_EVENT_FCI_ADDREF(pfci) Z_ADDREF_P(pfci_dst->function_name)
#define PHP_EVENT_COPY_FCALL_INFO(pfci_dst, pfcc_dst, pfci, pfcc)                                \
    if (ZEND_FCI_INITIALIZED(*pfci)) {                                                           \
                                                                                                 \
        memcpy(pfci_dst, pfci, sizeof(zend_fcall_info));                                         \
        memcpy(pfcc_dst, pfcc, sizeof(zend_fcall_info_cache));                                   \
                                                                                                 \
        PHP_EVENT_FCI_ADDREF(pfci_dst);                                                          \
    } else {                                                                                     \
        pfci_dst = NULL;                                                                         \
        pfcc_dst = NULL;                                                                         \
    }

void fill_file_handle(file_handle_data *handleData, char *filename,
                      zend_fcall_info *fci,
                      zend_fcall_info_cache *fcc) {
    handleData->filename = filename;
    uv_cb_type tt = {};
//    handleData->php_cb_data = {};//emalloc(sizeof(uv_cb_type));
    LOG("fill_file_handle %p %p %s\n", &handleData->php_cb_data.fci, &fci, filename);
    if (ZEND_FCI_INITIALIZED(*fci)) {
        memcpy(&tt.fci, fci, sizeof(zend_fcall_info));
        memcpy(&tt.fcc, fcc, sizeof(zend_fcall_info_cache));
        Z_ADDREF_P(&tt.fci.function_name);
        if (fci->object) {
            GC_ADDREF(fci->object);
        }
    }

    memcpy(&handleData->php_cb_data, &tt, sizeof(uv_cb_type));

}