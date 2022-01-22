//
// Created by admin on 13.12.2021.
//

#ifndef FILEIO_DB_ASYNC_OVERRIDE_H
#define FILEIO_DB_ASYNC_OVERRIDE_H

#include "../common/callback_interface.h"

enum DB_TYPE{
    MYSQL_DB,
    PGSQL_DB,
};

typedef struct {
    uv_cb_type cb;
    enum DB_TYPE type;
    zval * db_handle;
} db_type_t;

#endif //FILEIO_DB_ASYNC_OVERRIDE_H
