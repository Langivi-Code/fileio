//
// Created by admin on 16.09.2021.
//

#ifndef FILEIO_FILL_EVENT_HANDLE_H
#define FILEIO_FILL_EVENT_HANDLE_H

#include <zend_API.h>
#include "callback_interface.h"

void fill_event_handle(void *handle_data, const zend_fcall_info *fci, const zend_fcall_info_cache *fcc, uv_cb_type *uv);
#endif //FILEIO_FILL_EVENT_HANDLE_H
