//
// Created by admin on 14.09.2021.
//

#ifndef FILEIO_CALLBACK_INTERFACE_H
#define FILEIO_CALLBACK_INTERFACE_H
typedef struct {
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
} uv_cb_type;
#endif //FILEIO_CALLBACK_INTERFACE_H
