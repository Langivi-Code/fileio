//
// Created by admin on 16.09.2021.
//

#ifndef FILEIO_FILL_EVENT_HANDLE_H
#define FILEIO_FILL_EVENT_HANDLE_H

#include <zend_API.h>
#include "callback_interface.h"
uv_cb_type * init_cb_p(const zend_fcall_info *fci, const zend_fcall_info_cache *fcc);
void init_cb(const zend_fcall_info *fci, const zend_fcall_info_cache *fcc, uv_cb_type * cb_handle);
void fill_event_handle(void *handle_data, const zend_fcall_info *fci, const zend_fcall_info_cache *fcc);
#endif //FILEIO_FILL_EVENT_HANDLE_H
