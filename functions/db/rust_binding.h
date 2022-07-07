//
// Created by admin on 22.06.2022.
//

#ifndef FILEIO_RUST_BINDING_H
#define FILEIO_RUST_BINDING_H

#include <stdbool.h>
#include "../common/callback_interface.h"
#include "db_async_override.h"
#include "php.h"

typedef struct {
    uv_cb_type cb;
    uv_cb_type cb_read;
    bool written;
    enum DB_TYPE type;
    void * db_handle;} cb_item;
#endif //FILEIO_RUST_BINDING_H
/usr/local/Cellar/php/8.1.2/