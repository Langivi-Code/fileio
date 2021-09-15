//
// Created by admin on 14.09.2021.
//

#ifndef FILEIO_SET_TIMEOUT_INTERFACE_H
#define FILEIO_SET_TIMEOUT_INTERFACE_H
typedef struct {
    unsigned long time;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
} timerData;
#endif //FILEIO_SET_TIMEOUT_INTERFACE_H
