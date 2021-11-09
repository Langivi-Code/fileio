//
// Created by admin on 16.09.2021.
//

#include <memory.h>
#include "fill_event_handle.h"

void fill_event_handle(void *handle_data, const zend_fcall_info *fci, const zend_fcall_info_cache *fcc, uv_cb_type *uv) {

    memcpy(((uv_handle_t *)handle_data)->data, init_cb(fci, fcc), sizeof(uv_cb_type));
}

uv_cb_type * init_cb(const zend_fcall_info *fci, const zend_fcall_info_cache *fcc) {
    uv_cb_type * cb_handle = (uv_cb_type *) emalloc(sizeof(uv_cb_type));
    memcpy(&(*cb_handle).fci, fci, sizeof(zend_fcall_info));
    memcpy(&(*cb_handle).fcc, fcc, sizeof(zend_fcall_info_cache));
    if (ZEND_FCI_INITIALIZED(*fci)) {
        Z_TRY_ADDREF((*cb_handle).fci.function_name);
        if (fci->object) {
            GC_ADDREF(fci->object);
        }
    }
    return cb_handle;
}