//
// Created by user on 25.10.21.
//
#include <php.h>
#include <zend_API.h>
#include <uv.h>
#include "../common/fill_event_handle.h"
#include "../../php_fileio.h"
#include "ext/standard/file.h"
#include "server.h"
#include "helpers.h"
#include "server_args_info.h"

#define SERVER_ID "#"
#define CLOSABLE "##"
static zend_long server_id = -1;
server_type php_servers[10];
char headers[] = "HTTP/1.1 200 OK\r\nserver: 0.0.0.0:8004\r\ndate: Wed, 27 Oct 2021 09:07:01 GMT\r\n\r\n";


unsigned long long timeout = 100000;
struct timeval tv;

void parse_fci_error(long error, const char *func_name) {
    printf("%s - ", func_name);
    switch (error) {
        case 0:
            printf("Function run\n");
            break;
        case -1:
            printf("Error in function execution\n");
            break;
        case -2:
            printf("Function not initialized\n");
            break;
    }
}

void parse_uv_event(int event, int status) {
    switch (event) {
        case UV_DISCONNECT:
            printf("UV_DISCONNECT event, status -  %d \n", status);
            break;
        case UV_CONNECT:
            printf("UV_CONNECT event, status -  %d \n", status);
            break;
        case UV_READABLE:
            printf("UV_READABLE event, status -  %d \n", status);
            break;
    }
}

void on_listen_client_event(uv_poll_t *handle, int status, int events) {
    GET_SERV_ID_FROM_EVENT_HANDLE();
    parse_uv_event(events, status);
    php_stream *clistream = NULL;
    zend_long error = 0;
    event_handle_item * event_handle = (event_handle_item *) handle->data;
    clistream = (php_stream *) event_handle->handle_data;
    if (php_stream_eof(clistream)) {
//        get_meta_data(clistream);
        uv_poll_stop(handle);
        zval * closable_zv;
        closable_zv = zend_read_property(event_handle->this->ce, event_handle->this, CLOSABLE, sizeof(CLOSABLE)-1, 0, NULL);
       long closable = Z_LVAL_P(closable_zv);
        printf("closable %dl\n",closable);
        if (events == 5 && ZEND_FCI_INITIALIZED(php_servers[cur_id].on_disconnect.fci)) {
            if (zend_call_function(&php_servers[cur_id].on_disconnect.fci, &php_servers[cur_id].on_connect.fcc) !=
                SUCCESS) {
                error = -1;
            }
        } else {
            error = -2;
        }
        parse_fci_error(error, "on disconnect");
        php_stream_free(clistream, PHP_STREAM_FREE_KEEP_RSRC |
                                   (clistream->is_persistent ? PHP_STREAM_FREE_CLOSE_PERSISTENT
                                                             : PHP_STREAM_FREE_CLOSE));
        php_servers[cur_id].current_client_stream = NULL;
        efree(handle);
        return;
    }
    zend_string * contents = NULL;
    zval retval;
    //TODO copy to zval containing data
    zval args[1];
    contents = php_stream_read_to_str(clistream, 1024);
    int position = php_stream_tell(clistream);
    printf("Data pointer pos -  %d, Server id is %ld\n", position, cur_id);
    ZVAL_STR(&args[0], contents);
    php_servers[cur_id].on_data.fci.param_count = 1;
    php_servers[cur_id].on_data.fci.params = args;
    php_servers[cur_id].on_data.fci.retval = &retval;
    if (ZEND_FCI_INITIALIZED(php_servers[cur_id].on_data.fci)) {
        if (zend_call_function(&php_servers[cur_id].on_data.fci, &php_servers[cur_id].on_data.fcc) != SUCCESS) {
            error = -1;
        }
    } else {
        error = -2;
    }
    parse_fci_error(error, "on data");

}


void on_listen_server_for_clients(uv_poll_t *handle, int status, int events) {
    GET_SERV_ID_FROM_EVENT_HANDLE();
    parse_uv_event(events, status);
    zend_long flags = STREAM_PEEK;
    zend_string * errstr = NULL;
    php_stream *clistream = NULL;

    int this_fd;
    php_stream_xport_accept(php_servers[cur_id].server_stream, &clistream, NULL, NULL, NULL, &tv, &errstr);

    if (!clistream) {
        php_error_docref(NULL, E_ERROR, "Accept failed: %s", errstr ? ZSTR_VAL(errstr) : "Unknown error");
    }
    php_servers[cur_id].current_client_stream = clistream;
    int ret = clistream->ops->set_option(clistream, PHP_STREAM_OPTION_BLOCKING, 0, NULL);
    uv_poll_t *cli_handle = emalloc(sizeof(uv_poll_t));
    int cast_result = _php_stream_cast(clistream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL,
                                       (void *) &this_fd, 1);

    if (cast_result == SUCCESS && ret == SUCCESS) {
        uv_poll_init_socket(FILE_IO_GLOBAL(loop), cli_handle, this_fd);
        event_handle_item handleItem = {.cur_id=cur_id, .handle_data=clistream, .this=((event_handle_item *) handle->data)->this};
        memcpy(cli_handle->data, &handleItem, sizeof(event_handle_item));
        uv_poll_start(cli_handle, UV_READABLE | UV_DISCONNECT, on_listen_client_event);
    } else {
        php_error_docref(NULL, E_ERROR, "Accept failed: %s", errstr ? ZSTR_VAL(errstr) : "Unknown error");
    }
    zend_long error;
    zval retval;
    php_servers[cur_id].on_connect.fci.retval = &retval;
    //    zend_call_method_with_1_params(NULL, NULL, NULL, "print_r", &retval, &dstr);
    if (ZEND_FCI_INITIALIZED(php_servers[cur_id].on_connect.fci)) {
        if (zend_call_function(&php_servers[cur_id].on_connect.fci, &php_servers[cur_id].on_connect.fcc) != SUCCESS) {
            error = -1;
        }
    } else {
        error = -2;
    }
    parse_fci_error(error, "on connect");
    php_stream_xport_sendto(clistream, headers, sizeof(headers) - 1, (int) flags, NULL, 0);
    //TODO remove after debug
    php_stream_write(clistream, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"));

}

void sig_cb(void *arg) {
    printf("sig int\n");
}

PHP_FUNCTION (server) {
    char *host = NULL;
    size_t host_len = 0;
    size_t ret_sz;
    zend_long port;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    zend_fcall_info fci = empty_fcall_info;
    zval * zerrno = NULL, *zerrstr = NULL, *zcontext = NULL;
    ZEND_PARSE_PARAMETERS_START(1, 3)
            Z_PARAM_LONG(port)
            Z_PARAM_OPTIONAL
            Z_PARAM_STRING_OR_NULL(host, host_len)
            Z_PARAM_FUNC(fci, fcc);

    ZEND_PARSE_PARAMETERS_END();
    int err = 0;
    zend_long flags = STREAM_XPORT_BIND | STREAM_XPORT_LISTEN;
    zend_string * errstr = NULL;
    php_stream_context *context = NULL;

    const char *temp_host = create_host(host, host_len, port, &ret_sz);
    char full_host[ret_sz];
    strcpy(full_host, temp_host);
    efree(temp_host);
    printf("----- Running on port %d by full dns %s -----\n", port, full_host);

#ifdef PHP_WIN32
    tv.tv_sec = (long)(timeout / 1000000);
    tv.tv_usec = (long)(timeout % 1000000);
#else
    tv.tv_sec = timeout / 1000000;
    tv.tv_usec = timeout % 1000000;
#endif
    ++server_id;
    zend_long cur_id = server_id;
    zval id;
    ZVAL_LONG(&id, server_id);
    zend_update_property(FILE_IO_GLOBAL(server_class), Z_OBJ_P(ZEND_THIS), "#", sizeof("#") - 1, &id);
    uv_poll_t *handle = emalloc(sizeof(uv_poll_t));
//    context = php_stream_context_from_zval(NULL, flags & PHP_FILE_NO_DEFAULT_CONTEXT);
//    if (context) {
//        GC_ADDREF(context->res);
//    }
    printf("Server id is %ld\n", Z_LVAL(id));
    php_servers[cur_id].server_stream = _php_stream_xport_create(full_host, ret_sz, REPORT_ERRORS,
                                                                 STREAM_XPORT_SERVER | (int) flags,

                                                                 NULL, NULL, NULL, &errstr, &err);
//       get_meta_data(php_servers[cur_id].server_stream);
    if (php_servers[cur_id].server_stream == NULL) {
        php_error_docref(NULL, E_WARNING, "Unable to connect  %s\n",
                         errstr == NULL ? "Unknown error" : ZSTR_VAL(errstr));
    }
    printf("Stream errors %d\n", err);
    int ret = php_servers[cur_id].server_stream->ops->set_option(php_servers[cur_id].server_stream,
                                                                 PHP_STREAM_OPTION_BLOCKING, 0, NULL);
    printf("Set non block result: %d\n", ret);
    int cast_result = _php_stream_cast(php_servers[cur_id].server_stream,
                                       PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL,
                                       (void *) &php_servers[cur_id].server_fd, 1);
    printf("Server FD is: %d\n", php_servers[cur_id].server_fd);
    memcpy(&php_servers[cur_id].on_connect.fci, &fci, sizeof(zend_fcall_info));
    memcpy(&php_servers[cur_id].on_connect.fcc, &fcc, sizeof(zend_fcall_info_cache));
    if (ZEND_FCI_INITIALIZED(fci)) {
        Z_TRY_ADDREF(php_servers[cur_id].on_connect.fci.function_name);
        if (php_servers[cur_id].on_connect.fci.object) {
            GC_ADDREF(php_servers[cur_id].on_connect.fci.object);
        }
    }
    if (SUCCESS == cast_result && ret == 1 && php_servers[cur_id].server_fd != -1) {
        uv_poll_init_socket(FILE_IO_GLOBAL(loop), handle, php_servers[cur_id].server_fd);
        uv_signal_t *sig_handle = emalloc(sizeof(uv_signal_t));
        uv_signal_init(FILE_IO_GLOBAL(loop), sig_handle);
        uv_cb_type uv = {};
//        LOG("size of timeout handler %lu, fci  %lu \n\n", sizeof *idle_type, sizeof *fci);
        php_servers[cur_id].connect_handle = handle->data = (uv_cb_type *) emalloc(sizeof(uv_cb_type));
        fill_event_handle(handle, &fci, &fcc, &uv);
        event_handle_item handleItem = {.cur_id=cur_id, .this=Z_OBJ_P(ZEND_THIS)};
        memcpy(handle->data, &handleItem, sizeof(event_handle_item));
        uv_poll_start(handle, UV_READABLE, on_listen_server_for_clients);
//        uv_signal_start(sig_handle, (uv_signal_cb) sig_cb, SIGINT);
    } else {
        php_error_docref(NULL, E_WARNING, "Unable to get fd   %s\n", "Unknown error");
    }
    printf("Server start up finished \n\n");
    puts("------------------------------------------------------------\n");
}


PHP_FUNCTION (server_on_data) {
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    zend_fcall_info fci = empty_fcall_info;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_FUNC(fci, fcc);ZEND_PARSE_PARAMETERS_END();
    GET_SERV_ID();
    printf("Server_on_data  Server id is %ld\n", cur_id);
    memcpy(&php_servers[cur_id].on_data.fci, &fci, sizeof(zend_fcall_info));
    memcpy(&php_servers[cur_id].on_data.fcc, &fcc, sizeof(zend_fcall_info_cache));
//    php_servers[cur_id].on_data.fcc.object = php_servers[cur_id].on_data.fci.object = Z_OBJ_P(ZEND_THIS);
//    php_servers[cur_id].on_data.fcc.called_scope = Z_OBJCE_P(ZEND_THIS);
//    php_servers[cur_id].on_data.fcc.calling_scope = Z_OBJCE_P(ZEND_THIS);
    if (ZEND_FCI_INITIALIZED(fci)) {
        Z_TRY_ADDREF(php_servers[cur_id].on_data.fci.function_name);
        if (php_servers[cur_id].on_data.fci.object) {
            GC_ADDREF(php_servers[cur_id].on_data.fci.object);
        }
    } else {
        zend_throw_error(NULL, "on data is not initialized");
    }
}

PHP_FUNCTION (server_on_disconnect) {
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    zend_fcall_info fci = empty_fcall_info;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_FUNC(fci, fcc);ZEND_PARSE_PARAMETERS_END();
    GET_SERV_ID();
    printf("Server_on_disconnect  Server id is %ld\n", cur_id);
    memcpy(&php_servers[cur_id].on_disconnect.fci, &fci, sizeof(zend_fcall_info));
    memcpy(&php_servers[cur_id].on_disconnect.fcc, &fcc, sizeof(zend_fcall_info_cache));
    if (ZEND_FCI_INITIALIZED(fci)) {
        Z_TRY_ADDREF(php_servers[cur_id].on_disconnect.fci.function_name);
        if (php_servers[cur_id].on_disconnect.fci.object) {
            GC_ADDREF(php_servers[cur_id].on_disconnect.fci.object);
        }
    } else {
        zend_throw_error(NULL, "on disconnect is not initialized");
    }
}

PHP_FUNCTION (server_write) {
    char *data;
    zend_long data_len;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STRING(data, data_len)ZEND_PARSE_PARAMETERS_END();
    GET_SERV_ID();
    zend_update_property(Z_OBJ_P(ZEND_THIS)->ce, Z_OBJ_P(ZEND_THIS), CLOSABLE, sizeof(CLOSABLE)-1, 0);
    php_stream_write(php_servers[cur_id].current_client_stream, data, data_len);
}

PHP_FUNCTION (server_on_error) {
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    zend_fcall_info fci = empty_fcall_info;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_FUNC(fci, fcc);ZEND_PARSE_PARAMETERS_END();
    GET_SERV_ID();
    printf("Server_on_error  Server id is %ld\n", cur_id);
    memcpy(&php_servers[cur_id].on_error.fci, &fci, sizeof(zend_fcall_info));
    memcpy(&php_servers[cur_id].on_error.fcc, &fcc, sizeof(zend_fcall_info_cache));
    if (ZEND_FCI_INITIALIZED(fci)) {
        Z_TRY_ADDREF(php_servers[cur_id].on_error.fci.function_name);
        if (php_servers[cur_id].on_error.fci.object) {
            GC_ADDREF(php_servers[cur_id].on_error.fci.object);
        }
    } else {
        zend_throw_error(NULL, "on error is not initialized");
    }
}

static const zend_function_entry class_Server_methods[] = {
        ZEND_ME_MAPPING(__construct, server, arginfo_server, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(on_data, server_on_data, arginfo_server_event_handler, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(on_disconnect, server_on_disconnect, arginfo_server_event_handler, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(on_error, server_on_error, arginfo_server_event_handler, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(write, server_write, arginfo_server_write, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

zend_class_entry *register_class_Server(void) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Server", class_Server_methods);
    FILE_IO_GLOBAL(server_class) = zend_register_internal_class_ex(&ce, NULL);
    FILE_IO_GLOBAL(server_class)->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES | ZEND_ACC_NOT_SERIALIZABLE;
    zend_declare_property_long(FILE_IO_GLOBAL(server_class), SERVER_ID, sizeof(SERVER_ID) - 1, server_id,
                               ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_bool(FILE_IO_GLOBAL(server_class), CLOSABLE, sizeof(CLOSABLE) - 1, 1,
                               ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    return FILE_IO_GLOBAL(server_class);
}