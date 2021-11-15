//
// Created by admin on 15.11.2021.
//
#include <zend_API.h>
#include "../http/request.h"

#ifndef FILEIO_RESPONSE_CLASS_H
#define FILEIO_RESPONSE_CLASS_H

typedef struct _response_obj {
    zend_object std;

    key_value_collection headers;
    unsigned short status_code;
    zend_object *server;

} response_obj;

#endif //FILEIO_RESPONSE_CLASS_H
