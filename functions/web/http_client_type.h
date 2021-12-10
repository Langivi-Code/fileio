//
// Created by admin on 10.12.2021.
//

#ifndef FILEIO_HTTP_CLIENT_TYPE_H
#define FILEIO_HTTP_CLIENT_TYPE_H

#include <uv.h>
#include "php_streams.h"
#include "pointer_to_free.h"

typedef struct {
    php_stream *current_stream;
    int current_fd;
    uv_buf_t write_buf;
    uv_timer_t * close_timer;
    pntrs_to_free pointers;
} http_client_type;
#endif //FILEIO_HTTP_CLIENT_TYPE_H
