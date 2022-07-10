//
// Created by admin on 22.06.2022.
//

#ifndef FILEIO_RUST_BINDING_H
#define FILEIO_RUST_BINDING_H

#include <stdbool.h>
#include "../common/callback_interface.h"
#include "php.h"

enum DB_TYPE{
    MYSQL_DB,
    PGSQL_DB,
};

typedef struct cb_item{
    uv_cb_type cb;
    uv_cb_type cb_read;
    bool read;
    bool written;
    enum DB_TYPE type;
    zval db_handle;
    void * conn;
} cb_item;
#endif //FILEIO_RUST_BINDING_H
