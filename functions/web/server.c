//
// Created by user on 25.10.21.
//
#include <php.h>
#include <zend_API.h>
#include <uv.h>
#include "../../php_fileio.h"
#include "ext/standard/file.h"
#include "server.h"

php_stream *stream = NULL;
long i=0;
void poll_cb(uv_poll_t *handle, int status, int events) {
    php_stream *clistream = NULL;
    struct timeval tv;
    zend_long flags = 0;
    zend_string *errstr = NULL;
    printf("%d,%d\n", status, events);

    unsigned long long timeout = 100000000;

#ifdef PHP_WIN32
    tv.tv_sec = (long)(timeout / 1000000);
    tv.tv_usec = (long)(timeout % 1000000);
#else
    tv.tv_sec = timeout / 1000000;
    tv.tv_usec = timeout % 1000000;
#endif

    php_stream_xport_accept(stream, &clistream, NULL, NULL, NULL, &tv, &errstr);

    if (!clistream) {
        php_error_docref(NULL, E_WARNING, "Accept failed: %s", errstr ? ZSTR_VAL(errstr) : "Unknown error");
    }
    int ret = clistream->ops->set_option(stream, PHP_STREAM_OPTION_BLOCKING, 0, NULL);
    printf("accented %d", ret);

    php_stream_xport_sendto(clistream, "HTTP/1.1 200 OK\n", strlen("HTTP/1.1 200 OK\n"), (int) flags, NULL, 0);
    i++;
    printf("req %ld", i);
//    php_stream_write(clistream, "HTTP/1.1 200 OK\n", strlen("HTTP/1.1 200 OK\n"));
    php_stream_free(clistream,
                    PHP_STREAM_FREE_KEEP_RSRC |
                    (stream->is_persistent ? PHP_STREAM_FREE_CLOSE_PERSISTENT : PHP_STREAM_FREE_CLOSE));
//    uv_poll_stop(handle);
//    php_stream_xport_shutdown(stream, 0);
}


PHP_FUNCTION (server) {

    char *host;
    size_t host_len;
    zval *zerrno = NULL, *zerrstr = NULL, *zcontext = NULL;

    int err = 0;
    zend_long flags = STREAM_XPORT_BIND | STREAM_XPORT_LISTEN;
    zend_string *errstr = NULL;
    php_stream_context *context = NULL;
    php_socket_t this_fd;
    uv_poll_t *handle = emalloc(sizeof(uv_poll_t));
    context = php_stream_context_from_zval(NULL, flags & PHP_FILE_NO_DEFAULT_CONTEXT);
    if (context) {
        GC_ADDREF(context->res);
    }
    stream = _php_stream_xport_create("tcp://0.0.0.0:8003", 18, REPORT_ERRORS, STREAM_XPORT_SERVER | (int) flags,
                                      NULL, NULL, context, &errstr, &err);
    if (stream == NULL) {
        php_error_docref(NULL, E_WARNING, "Unable to connect  %s\n",
                         errstr == NULL ? "Unknown error" : ZSTR_VAL(errstr));
    }
//    printf("stream errors %s", ZSTR_VAL(errstr));
    printf("stream errors %d\n", err);
    int ret = stream->ops->set_option(stream, PHP_STREAM_OPTION_BLOCKING, 0, NULL);
    printf("set non block result: %d\n", ret);
    int cast_result = _php_stream_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL,
                                       (void *) &this_fd, 1);
    printf("FD is  : %d\n", this_fd);
    if (SUCCESS == cast_result && this_fd != -1) {
        uv_poll_init_socket(FILE_IO_GLOBAL(loop), handle, this_fd);
        uv_poll_start(handle, UV_READABLE | UV_DISCONNECT, poll_cb);

    }
    printf("cast result: %d\n", cast_result);

    //TODO create TCP server
    //TODO listen to a Port
    //TODO poll on connections
}