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
    bool sent;
    zend_object *server;
    unsigned long long current_client;

} response_obj;

response_obj *responseObj_from_zend_obj(zend_object *obj);
#endif //FILEIO_RESPONSE_CLASS_H
