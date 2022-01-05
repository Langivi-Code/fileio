//
// Created by user on 01.10.21.
//

#include "file_interface.h"
#include "../../constants.h"
#include <uv.h>

#define LOG_TAG "file_handles"


CREATE_HANDLE_LIST(fs, file_handle_data);

void close_cb(uv_fs_t *req) {
    long result = req->result;

    if (result == -1) {
        fprintf(stderr, "Error closing file: %s.\n",
                uv_strerror(result));
    }

    fs_id_t *fs_id = (fs_id_t *) req->data;
    fs_id_item_t *fs_handle = find_fs_handle(fs_handle_map, fs_id->id);
    if (fs_handle) {
        fs_close_reqs_t *requests_to_close = &fs_handle->handle->close_requests;
        uv_fs_req_cleanup(req);
        if (requests_to_close->write_req != NULL) {
            uv_fs_req_cleanup(requests_to_close->write_req);
            efree(requests_to_close->write_req);
        }
        if (requests_to_close->open_req != NULL) {
            uv_fs_req_cleanup(requests_to_close->open_req);
            efree(requests_to_close->open_req);
        }
        if (requests_to_close->status_req != NULL) {
            uv_fs_req_cleanup(requests_to_close->status_req);
            efree(requests_to_close->status_req);
        }
        if (requests_to_close->read_req != NULL) {
            uv_fs_req_cleanup(requests_to_close->read_req);
            efree(fs_handle->handle->buffer.base);
            efree(requests_to_close->read_req);
        }
        remove_fs_handle(fs_handle_map, fs_handle->handle_id);
        efree(fs_handle->handle);
        efree(req);
        efree(fs_id);
        printf("Close CB: Successfuly closed file.\n");
    }
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
