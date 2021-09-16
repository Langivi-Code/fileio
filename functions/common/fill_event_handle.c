//
// Created by admin on 16.09.2021.
//

#include <memory.h>
#include "fill_event_handle.h"

void fill_event_handle(void *handle_data, const zend_fcall_info *fci, const zend_fcall_info_cache *fcc, uv_cb_type *uv) {
    memcpy(&(*uv).fci, fci, sizeof(zend_fcall_info));
    memcpy(&(*uv).fcc, fcc, sizeof(zend_fcall_info_cache));
    memcpy(handle_data, uv, sizeof(uv_cb_type));

    if (ZEND_FCI_INITIALIZED(*fci)) {
        Z_TRY_ADDREF((*uv).fci.function_name);
        if (fci->object) {
            GC_ADDREF(fci->object);
        }
    }
}