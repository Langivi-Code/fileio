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
#include "../common/struct.h"
#include "server_args_info.h"
#define LOG_TAG "HTTP SERVER"
#define SERVER_ID "#"
#define CLOSABLE "##"

/*          TODO
 * 1. Add header parser
 * 2. Add addition to global variables
 * 3. Use only one Request / Response
 * 4. Create Request class
 * 5. Create Response class
 * 6. Generate Response headers
 * */

static zend_long server_id = -1;
server_type php_servers[10];

CREATE_HANDLE_LIST(client_stream, client_type);
char headers[] = "HTTP/1.1 200 OK\r\nserver: 0.0.0.0:8004\r\ndate: Wed, 27 Oct 2021 09:07:01 GMT\r\n\r\n";


unsigned long long timeout = 100000;
struct timeval tv;

void sig_cb(uv_poll_t *handle, int status, int events) {
    LOG("sig int\n");
    uv_poll_stop(handle);
    //TODO STOP ALL CLIENT HANDLES
    //STOP SERVER HANDLES
    //FREE CALLBACKS

}

void on_ready_to_write(uv_poll_t *handle, int status, int events) {

    GET_SERV_ID_FROM_EVENT_HANDLE();
    event_handle_item *event_handle = (event_handle_item *) handle->data;
    unsigned long long id = (unsigned long long) event_handle->handle_data;
    client_stream_id_item_t *client = find_client_stream_handle(php_servers[cur_id].client_stream_handle_map, id);
    php_stream *clistream = client->handle->current_stream;
    if (clistream != NULL) {

        if (php_servers[cur_id].write_buf.len > 1) {
            LOG("**Data get from buffer: %s  sz:%zu **\n", php_servers[cur_id].write_buf.base,
                   php_servers[cur_id].write_buf.len);
            php_stream_write(clistream, php_servers[cur_id].write_buf.base,
                             php_servers[cur_id].write_buf.len);
            efree(php_servers[cur_id].write_buf.base);
            php_servers[cur_id].write_buf.len = 1;
            LOG("**Data get from buffer: %s  sz:%zu **\n", php_servers[cur_id].write_buf.base,
                   php_servers[cur_id].write_buf.len);
        }

        if (client->handle->write_buf.len > 1) {
            LOG("**Data get from buffer TO CLIENT %lld: %s  sz:%zu**\n", client->handle_id,
                   client->handle->write_buf.base,
                   client->handle->write_buf.len);
            php_stream_write(clistream, client->handle->write_buf.base,
                             client->handle->write_buf.len);
            efree(client->handle->write_buf.base);
            client->handle->write_buf.len = 1;
        }
        zval rv1;
        ZVAL_BOOL(&rv1, 1);
        zend_update_property(event_handle->this->ce, event_handle->this, PROP(CLOSABLE), &rv1);
    } else {
        puts("Client handle is empty");
    }

}

static unsigned long long current_client = -1;

void on_listen_client_event(uv_poll_t *handle, int status, int events) {
    GET_SERV_ID_FROM_EVENT_HANDLE();
    parse_uv_event(events, status);
    event_handle_item *event_handle = (event_handle_item *) handle->data;
    unsigned long long id = current_client = (unsigned long long) event_handle->handle_data;
    client_stream_id_item_t *client = find_client_stream_handle(php_servers[cur_id].client_stream_handle_map, id);
    php_stream *clistream = client->handle->current_stream;

    zend_long error = 0;
    zval * closable_zv;
    long closable;
    closable_zv = zend_read_property(event_handle->this->ce, event_handle->this, CLOSABLE, sizeof(CLOSABLE) - 1, 0,
                                     NULL);
    closable = Z_TYPE_INFO_P(closable_zv);
    if (events & UV_WRITABLE) {
        on_ready_to_write(handle, status, events);
    }
    if (events & UV_DISCONNECT)
        LOG("-------------- READY FOR DISCONNECT(%d)----------\n", events);
    if (php_stream_eof(clistream) && client->handle->write_buf.len <= 1) {
//        get_meta_data(clistream);
        uv_poll_stop(handle);
        if (events == 5 && ZEND_FCI_INITIALIZED(php_servers[cur_id].on_disconnect.fci)) {
            if (zend_call_function(&php_servers[cur_id].on_disconnect.fci, &php_servers[cur_id].on_disconnect.fcc) !=
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
        remove_client_stream_handle(php_servers[cur_id].client_stream_handle_map, id);
        efree(client->handle);
        efree(handle->data);
        efree(handle);
        php_servers[cur_id].clients_count--;
        return;
    }

    if (events & UV_READABLE) {

        zend_string * contents = NULL;
        zval retval;
        zval args[1];
        LOG("Unread bytes  %lld\n", clistream->writepos - clistream->readpos, cur_id);
        zval * buf_size_zv = zend_read_property(event_handle->this->ce, event_handle->this, PROP("readBufferSize"), 0, NULL);
        zend_long buf_size =  Z_LVAL_P(buf_size_zv);
        contents = php_stream_read_to_str(clistream, buf_size); //TODO RESTORE SIZE TO READ OR USE ANOTHER READ FUCNTION
        int position = php_stream_tell(clistream);
//        get_meta_data(clistream);
        LOG("Data pointer pos -  %d, Server id is %ld\n", position, cur_id);
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
        current_client = -1; //something with client id on write in connect
        parse_fci_error(error, "on data");
        closable_zv = zend_read_property(event_handle->this->ce, event_handle->this, CLOSABLE, sizeof(CLOSABLE) - 1, 0,
                                         NULL);
        closable = Z_TYPE_INFO_P(closable_zv);
        LOG("closable %s\n", closable == IS_TRUE ? "true" : "false");
    }

}


void on_listen_server_for_clients(uv_poll_t *handle, int status, int events) {
    GET_SERV_ID_FROM_EVENT_HANDLE();
    parse_uv_event(events, status);
    zend_string * errstr = NULL;
    zend_string * textaddr = NULL;
    php_stream *clistream = NULL;


    php_stream_xport_accept(php_servers[cur_id].server_stream, &clistream, &textaddr, NULL, NULL, NULL, &errstr);

    if (!clistream) {
        php_error_docref(NULL, E_ERROR, "Accept failed: %s", errstr ? ZSTR_VAL(errstr) : "Unknown error");
    }
    zend_result cast_result;
    int this_fd = cast_to_fd(clistream, &cast_result);
    int ret = set_non_blocking(clistream);
    uv_poll_t *cli_handle = emalloc(sizeof(uv_poll_t));


    LOG("non block %d cast %d\n", ret, cast_result);
    LOG("Client address : %s\n", textaddr ? ZSTR_VAL(textaddr) : "no address");
    LOG("New connection accepted fd is %d\n ", this_fd);

    if (cast_result == SUCCESS && ret == SUCCESS) {
        php_servers[cur_id].clients_count++;
        client_type *que_cli_handle = emalloc(sizeof(client_type));
        memset(que_cli_handle, 0, sizeof(client_type));
        que_cli_handle->current_stream = clistream;
        clistream = NULL;
        que_cli_handle->current_fd = this_fd;
        que_cli_handle->client_handle = cli_handle;
        this_fd = -1;
        unsigned long long id = add_client_stream_handle(php_servers[cur_id].client_stream_handle_map, que_cli_handle);
        uv_poll_init_socket(FILE_IO_GLOBAL(loop), cli_handle, que_cli_handle->current_fd); // if the same what to do?
//        event_handle_item handleItem = {.cur_id=cur_id, .handle_data=(void *) id, .this=((event_handle_item *) handle->data)->this};
        event_handle_item *handleItem = emalloc(sizeof(event_handle_item));
        handleItem->cur_id = cur_id;
        handleItem->handle_data = (void *) id;
        handleItem->this = ((event_handle_item *) handle->data)->this;
        cli_handle->data = handleItem;
        zend_update_property_string(FILE_IO_GLOBAL(server_class), handleItem->this, PROP("clientAddress"), textaddr);
        uv_poll_start(cli_handle, UV_READABLE | UV_DISCONNECT | UV_WRITABLE, on_listen_client_event);
    } else {
        php_error_docref(NULL, E_ERROR, "Accept failed: %s", errstr ? ZSTR_VAL(errstr) : "Unknown error");
    }
    LOG("Client counts  by handles %d by counter %llu\n",
           count_client_stream_handles(php_servers[cur_id].client_stream_handle_map),
           php_servers[cur_id].clients_count);
//    exit(0);
    zend_long error = 0;
    zval retval;
    zval obj[1];
    ZVAL_OBJ(&obj[0], ((event_handle_item *) handle->data)->this);
    php_servers[cur_id].on_connect.fci.params = obj;
    php_servers[cur_id].on_connect.fci.param_count = 1;
    php_servers[cur_id].on_connect.fci.object = ((event_handle_item *) handle->data)->this;
    php_servers[cur_id].on_connect.fcc.object = ((event_handle_item *) handle->data)->this;
    php_servers[cur_id].on_connect.fcc.called_scope = ((event_handle_item *) handle->data)->this->ce;
    php_servers[cur_id].on_connect.fcc.calling_scope = ((event_handle_item *) handle->data)->this->ce;
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
//    zval_ptr_dtor(&retval);
//    zval_ptr_dtor(obj);
//    php_stream_xport_sendto(clistream, headers, sizeof(headers) - 1, (int) flags, NULL, 0);
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
    LOG("----- Running on port %d by full dns %s -----\n", port, full_host);

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
    zend_object * this = Z_OBJ_P(ZEND_THIS);
    zend_update_property_long(FILE_IO_GLOBAL(server_class), this, PROP("#"), server_id);
    zend_update_property_string(FILE_IO_GLOBAL(server_class), this, PROP("serverAddress"),
                                host == NULL ? "0.0.0.0" : host);
    zend_update_property_long(FILE_IO_GLOBAL(server_class), this, PROP("serverPort"), port);
    uv_poll_t *handle = emalloc(sizeof(uv_poll_t));  //TODO FREE on server shutdown
//    context = php_stream_context_from_zval(NULL, flags & PHP_FILE_NO_DEFAULT_CONTEXT);
//    if (context) {
//        GC_ADDREF(context->res);
//    }
    LOG("Server id is %ld\n", Z_LVAL(id));
    php_servers[cur_id].server_stream = _php_stream_xport_create(full_host, ret_sz, REPORT_ERRORS,
                                                                 STREAM_XPORT_SERVER | (int) flags,

                                                                 NULL, NULL, NULL, &errstr, &err);
//       get_meta_data(php_servers[cur_id].server_stream);
    if (php_servers[cur_id].server_stream == NULL) {
        php_error_docref(NULL, E_WARNING, "Unable to connect  %s\n",
                         errstr == NULL ? "Unknown error" : ZSTR_VAL(errstr));
    }

    LOG("Stream errors %d\n", err);
    int ret = set_non_blocking(php_servers[cur_id].server_stream);
    LOG("Set non block result: %d\n", ret);
    zend_result cast_result;
    php_servers[cur_id].server_fd = cast_to_fd(php_servers[cur_id].server_stream, &cast_result);
    LOG("Server FD is: %d\n", php_servers[cur_id].server_fd);


    init_cb(&fci, &fcc, &php_servers[cur_id].on_connect);

    if (SUCCESS == cast_result && ret == 1 && php_servers[cur_id].server_fd != -1) {
        uv_poll_init_socket(FILE_IO_GLOBAL(loop), handle, php_servers[cur_id].server_fd);
//        uv_signal_t *sig_handle = emalloc(sizeof(uv_signal_t)); //TODO FREE on server shutdown
//        uv_signal_init(FILE_IO_GLOBAL(loop), sig_handle);
//        uv_cb_type uv = {};
//        LOG("size of timeout handler %lu, fci  %lu \n\n", sizeof *idle_type, sizeof *fci);
        handle->data = (event_handle_item *) emalloc(sizeof(event_handle_item)); //TODO FREE on server shutdown
        php_servers[cur_id].clients_count = 0;
        event_handle_item handleItem = {.cur_id=cur_id, .this=Z_OBJ_P(ZEND_THIS)};
        memcpy(handle->data, &handleItem, sizeof(event_handle_item));
        uv_poll_start(handle, UV_READABLE, on_listen_server_for_clients);
//        uv_signal_start(sig_handle, (uv_signal_cb) sig_cb, SIGINT);
    } else {
        php_error_docref(NULL, E_WARNING, "Unable to get fd   %s\n", "Unknown error");
    }
    LOG("Server start up finished \n\n");
    puts("------------------------------------------------------------\n");
}


PHP_FUNCTION (server_on_data) {
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    zend_fcall_info fci = empty_fcall_info;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_FUNC(fci, fcc);ZEND_PARSE_PARAMETERS_END();
    GET_SERV_ID();
    LOG("Server_on_data  Server id is %lld\n", cur_id);
    init_cb(&fci, &fcc, &php_servers[cur_id].on_data);
//    php_servers[cur_id].on_data.fcc.object = php_servers[cur_id].on_data.fci.object = Z_OBJ_P(ZEND_THIS);
//    php_servers[cur_id].on_data.fcc.called_scope = Z_OBJCE_P(ZEND_THIS);
//    php_servers[cur_id].on_data.fcc.calling_scope = Z_OBJCE_P(ZEND_THIS);
    if (!ZEND_FCI_INITIALIZED(fci)) {
        zend_throw_error(NULL, "on data is not initialized");
    }
}

PHP_FUNCTION (server_on_disconnect) {
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    zend_fcall_info fci = empty_fcall_info;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_FUNC(fci, fcc);ZEND_PARSE_PARAMETERS_END();
    GET_SERV_ID();
    LOG("Server_on_disconnect  Server id is %ld\n", cur_id);
    init_cb(&fci, &fcc, &php_servers[cur_id].on_disconnect);
    if (!ZEND_FCI_INITIALIZED(fci)) {
        zend_throw_error(NULL, "on disconnect is not initialized");
    }
}

PHP_FUNCTION (server_write) {
    char *data;
    size_t data_len;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STRING(data, data_len)ZEND_PARSE_PARAMETERS_END();
    GET_SERV_ID();
    zval rv1;
    ZVAL_BOOL(&rv1, 0);
    LOG("clID: %lld\n", current_client);
    zend_update_property(Z_OBJ_P(ZEND_THIS)->ce, Z_OBJ_P(ZEND_THIS), CLOSABLE, sizeof(CLOSABLE) - 1, &rv1);
    unsigned int len = data_len + 1;
    //TODO APPEND data if not writeable
    if (current_client == FAILURE) {
        bool append = !(php_servers[cur_id].write_buf.len == 0 || php_servers[cur_id].write_buf.len == 1);
        if (php_servers[cur_id].write_buf.len == 0) {
            php_servers[cur_id].write_buf = uv_buf_init(emalloc(sizeof(char) * len), len);
            memset(php_servers[cur_id].write_buf.base, '\0', len);
        } else if (php_servers[cur_id].write_buf.len == 1) {
            php_servers[cur_id].write_buf.base = emalloc(sizeof(char) * len);
            php_servers[cur_id].write_buf.len = len;
            memset(php_servers[cur_id].write_buf.base, '\0', len);
        } else {
            php_servers[cur_id].write_buf.base = erealloc(php_servers[cur_id].write_buf.base,
                                                          sizeof(char) *
                                                          (php_servers[cur_id].write_buf.len + data_len));
            php_servers[cur_id].write_buf.len = php_servers[cur_id].write_buf.len + data_len;
        }
        if (append) {
            strncat(php_servers[cur_id].write_buf.base, data, data_len);
        } else {
            strncpy(php_servers[cur_id].write_buf.base, data, data_len);
        }

        LOG("Data set to buffer: %s, len %zu\n", php_servers[cur_id].write_buf.base,
               php_servers[cur_id].write_buf.len);
    } else {

        client_stream_id_item_t *client = find_client_stream_handle(php_servers[cur_id].client_stream_handle_map,
                                                                    current_client);
        bool append = !(client->handle->write_buf.len == 0 || client->handle->write_buf.len == 1);
        if (client->handle->write_buf.len == 0) {
            client->handle->write_buf = uv_buf_init(emalloc(sizeof(char) * len), len);
            memset(client->handle->write_buf.base, '\0', len);
        } else if (client->handle->write_buf.len == 1) {
            client->handle->write_buf.base = emalloc(sizeof(char) * len);
            client->handle->write_buf.len = len;
            memset(client->handle->write_buf.base, '\0', len);
        } else {
            client->handle->write_buf.base = erealloc(client->handle->write_buf.base,
                                                      sizeof(char) * (client->handle->write_buf.len + data_len));
            client->handle->write_buf.len = client->handle->write_buf.len + data_len;
        }
        if (append) {
            strncat(client->handle->write_buf.base, data, data_len);
        } else {
            strncpy(client->handle->write_buf.base, data, data_len);
        }

        LOG("Data set to buffer: %s, len %zu\n", client->handle->write_buf.base,
               client->handle->write_buf.len);
    }

}


PHP_FUNCTION (server_end) {
    char *data;
    zend_long data_len;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STRING(data, data_len)ZEND_PARSE_PARAMETERS_END();
    GET_SERV_ID();
    zval closable;
    if (current_client != FAILURE) {
        client_stream_id_item_t *client = find_client_stream_handle(php_servers[cur_id].client_stream_handle_map,
                                                                    current_client);
//    ZVAL_BOOL(&closable, 1);
        php_stream_free(client->handle->current_stream, PHP_STREAM_FREE_KEEP_RSRC |
                                                        (client->handle->current_stream->is_persistent
                                                         ? PHP_STREAM_FREE_CLOSE_PERSISTENT
                                                         : PHP_STREAM_FREE_CLOSE));
//    php_servers[cur_id].current_client_stream = NULL;
        zend_update_property(Z_OBJ_P(ZEND_THIS)->ce, Z_OBJ_P(ZEND_THIS), CLOSABLE, sizeof(CLOSABLE) - 1, &closable);
        remove_client_stream_handle(php_servers[cur_id].client_stream_handle_map, current_client);
        uv_poll_stop(client->handle->client_handle);
        efree(client->handle->client_handle->data);
        efree(client->handle->client_handle);
        efree(client->handle);
        php_servers[cur_id].clients_count--;
    }

}

PHP_FUNCTION (server_on_error) {
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    zend_fcall_info fci = empty_fcall_info;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_FUNC(fci, fcc);ZEND_PARSE_PARAMETERS_END();
    GET_SERV_ID();
    LOG("Server_on_error  Server id is %ld\n", cur_id);
    init_cb(&fci, &fcc, &php_servers[cur_id].on_error);
    if (!ZEND_FCI_INITIALIZED(fci)) {
        zend_throw_error(NULL, "on error is not initialized");
    }
}

ZEND_METHOD (Server, setReadBufferSize) {
    zend_long size = 1024;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_LONG(size);ZEND_PARSE_PARAMETERS_END();

    zend_update_property_long(Z_OBJ_P(ZEND_THIS)->ce, Z_OBJ_P(ZEND_THIS), PROP("readBufferSize"), size);

}

static const zend_function_entry class_Server_methods[] = {
        ZEND_ME_MAPPING(__construct, server, arginfo_server, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(on_data, server_on_data, arginfo_server_event_handler, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(on_disconnect, server_on_disconnect, arginfo_server_event_handler, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(on_error, server_on_error, arginfo_server_event_handler, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(write, server_write, arginfo_server_write, ZEND_ACC_PUBLIC)
        ZEND_ME(Server, setReadBufferSize, arginfo_server_setBuffer, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

zend_class_entry *register_class_Server(void) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Server", class_Server_methods);
    FILE_IO_GLOBAL(server_class) = zend_register_internal_class_ex(&ce, NULL);
    FILE_IO_GLOBAL(server_class)->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES | ZEND_ACC_NOT_SERIALIZABLE;
    zend_declare_property_long(FILE_IO_GLOBAL(server_class), PROP(SERVER_ID), server_id,
                               ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_bool(FILE_IO_GLOBAL(server_class), PROP(CLOSABLE), 1, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_string(FILE_IO_GLOBAL(server_class), PROP("serverAddress"), "",
                                 ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_long(FILE_IO_GLOBAL(server_class), PROP("serverPort"), 0,
                               ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_long(FILE_IO_GLOBAL(server_class), PROP("readBufferSize"), 1024,
                               ZEND_ACC_PUBLIC);
    zend_declare_property_string(FILE_IO_GLOBAL(server_class), PROP("clientAddress"), "",
                                 ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    return FILE_IO_GLOBAL(server_class);
}