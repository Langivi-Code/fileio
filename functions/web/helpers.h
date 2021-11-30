//
// Created by admin on 30.10.2021.
//

#ifndef FILEIO_HELPERS_H
#define FILEIO_HELPERS_H

#include "http_server.h"

char *create_host(const char *host, size_t host_len, zend_long port, size_t *str_len);
void get_meta_data(php_stream *stream);
void parse_fci_error(long error, const char *func_name);
void parse_uv_event(int event, int status);
int cast_to_fd(php_stream *stream,zend_result *cast_result);
int set_non_blocking(php_stream * stream);
bool fill_super_global(const unsigned char name, zval * value);

#endif //FILEIO_HELPERS_H
