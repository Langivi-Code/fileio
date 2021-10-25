#include <php.h>
#include <zend_API.h>
#include <uv.h>
#include "../common/fill_event_handle.h"
#include "../common/callback_interface.h"
#include "../../php_fileio.h"
#include "../../constants.h"

#include <streams/php_stream_context.h>
#include "ext/standard/file.h"


#include "../../fileio_arginfo.h"
#include "idle_interface.h"

#define LOG_TAG "fill_idle_handle_with_data"

void fill_idle_handle_with_data(
        uv_idle_t *idle_type,
        zend_fcall_info *fci,
        zend_fcall_info_cache *fcc
) {
    uv_cb_type uv = {};
    LOG("size of timeout handler %lu, fci  %lu \n\n", sizeof *idle_type, sizeof *fci);
    idle_type->data = (uv_cb_type *) emalloc(sizeof(uv_cb_type));
    fill_event_handle(idle_type, fci, fcc, &uv);
}


PHP_FUNCTION (idle) {
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    zval return_val;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_FUNC(fci, fcc)ZEND_PARSE_PARAMETERS_END();
    fci.retval = &return_val;
    fci.param_count = 0;
    uv_idle_t *idleHandle = emalloc(sizeof(uv_idle_t));

    uv_idle_init(FILE_IO_GLOBAL(loop), idleHandle);
    fill_idle_handle_with_data(idleHandle, &fci, &fcc);
    LOG("Setting idle ...\n");
    uv_idle_start(idleHandle, fn_idle);

    RETURN_NULL();
}

void poll_cb(uv_poll_t *handle, int status, int events) {
    printf("%d,%d", status, events);
    handle;
}


PHP_FUNCTION (server) {

    char *host;
    size_t host_len;
    zval * zerrno = NULL, *zerrstr = NULL, *zcontext = NULL;
    php_stream *stream = NULL;
    int err = 0;
    zend_long flags = STREAM_XPORT_BIND | STREAM_XPORT_LISTEN;
    zend_string * errstr = NULL;
    php_stream_context *context = NULL;
    php_socket_t this_fd;
    context = php_stream_context_from_zval(NULL, flags & PHP_FILE_NO_DEFAULT_CONTEXT);
    if (context) {
        GC_ADDREF(context->res);
    }
    if (zerrno) {
        ZEND_TRY_ASSIGN_REF_LONG(zerrno, 0);
    }
    if (zerrstr) {
        ZEND_TRY_ASSIGN_REF_EMPTY_STRING(zerrstr);
    }
    stream = _php_stream_xport_create("tcp://0.0.0.0:80", 16, REPORT_ERRORS, STREAM_XPORT_SERVER,
                                      NULL, NULL, context, &errstr, &err);
    int ret = stream->ops->set_option(stream, PHP_STREAM_OPTION_BLOCKING, 0, NULL);
    if (SUCCESS ==
        _php_stream_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL, (void *) &this_fd, 1) &&
        this_fd != -1) {
        uv_poll_t *handle;
        uv_poll_init_socket(FILE_IO_GLOBAL(loop), handle, this_fd);
        uv_poll_start(handle, UV_READABLE | UV_DISCONNECT, poll_cb);

    }
    //TODO create TCP server
    //TODO listen to a Port
    //TODO poll on connections
}
/* }}}*/