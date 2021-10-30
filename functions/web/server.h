//
// Created by user on 25.10.21.
//

#ifndef FILEIO_SERVER_H
#define FILEIO_SERVER_H

struct server_type {
    php_stream * server_stream;
    int server_fd;
    uv_cb_type on_data;
    uv_cb_type on_connect;
    uv_cb_type on_disconnect;
    uv_cb_type on_error;
    uv_poll_t * connect_handle;
    uv_poll_t * read_handle;
};

#endif //FILEIO_SERVER_H
