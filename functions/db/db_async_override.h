//
// Created by admin on 13.12.2021.
//

#ifndef FILEIO_DB_ASYNC_OVERRIDE_H
#define FILEIO_DB_ASYNC_OVERRIDE_H

#include "../common/callback_interface.h"
#include "db_types.h"

typedef struct {
    uv_cb_type cb;
    uv_cb_type cb_read;
	bool written;
    enum DB_TYPE type;
    zval * db_handle;
} db_type_t;

typedef struct {
    struct {
        uint_fast32_t id;
        db_type_t db_request_data;

    } * db_poll_item;
    bool inited; //true
    int fd; //12
} db_poll_queue;

#endif //FILEIO_DB_ASYNC_OVERRIDE_H
