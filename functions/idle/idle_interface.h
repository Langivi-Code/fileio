//
// Created by admin on 14.09.2021.
//

#ifndef FILEIO_SET_TIMEOUT_INTERFACE_H
#define FILEIO_SET_TIMEOUT_INTERFACE_H
typedef struct {
    unsigned long time;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
} idleData;

void fill_idle_handle_with_data(
        uv_idle_t *idle_type,
        zend_fcall_info *fci,
        zend_fcall_info_cache *fcc
);
#endif //FILEIO_SET_TIMEOUT_INTERFACE_H
