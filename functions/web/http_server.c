//
// Created by user on 25.10.21.
//
#include <php.h>
#include <zend_API.h>

#include <uv.h>
#include <stdnoreturn.h>
#include "../common/fill_event_handle.h"
#include "../../php_fileio.h"
#include "ext/standard/file.h"
#include "ext/standard/php_var.h"
#include "http_server.h"
#include "helpers.h"
#include "../common/struct.h"
#include "http_server_args_info.h"
#include "header.h"

#define LOG_TAG "HTTP SERVER"


/*          TODO
 * 1. Add header parser
 * 2. Add addition to global variables
 * 3. Use only one Request / Response
 * 4. Create Request class
 * 5. Create Response class
 * 6. Generate Response headers
 * */
CREATE_HANDLE_LIST(http_client_stream, http_client_type);

zend_class_entry *register_class_HttpRequest(void);

static zend_long server_id = -1;
server_type http_php_servers[10];


static unsigned long long timeout = 100000;
static struct timeval tv;

static void sig_cb(uv_poll_t *handle, int status, int events) {
    LOG("sig int\n");
    uv_poll_stop(handle);
    //TODO STOP ALL CLIENT HANDLES
    //STOP SERVER HANDLES
    //FREE CALLBACKS
}





static PHP_FUNCTION (server) {
    char *host = NULL;
    size_t host_len = 0;
    size_t ret_sz;
    zend_long port;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    zend_fcall_info fci = empty_fcall_info;
    HashTable options;
    zend_bool ssl = 0;
    zval * zerrno = NULL, *zerrstr = NULL, *zcontext = NULL;
    ZEND_PARSE_PARAMETERS_START(1, 3)
            Z_PARAM_LONG(port)
            Z_PARAM_OPTIONAL
            Z_PARAM_STRING_OR_NULL(host, host_len)
            Z_PARAM_ARRAY_HT_OR_NULL(options)ZEND_PARSE_PARAMETERS_END();
    int err = 0;
    zend_long flags = STREAM_XPORT_BIND | STREAM_XPORT_LISTEN;
    zend_string * errstr = NULL;
    php_stream_context *context = NULL;

    const char *temp_host = create_host(host, host_len, port, &ret_sz);
    char full_host[ret_sz + 6];
//    strcpy(full_host, ssl ? "ssl://":"tcp://");
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
    zend_update_property_long(FILE_IO_GLOBAL(http_server_class), this, PROP("#"), server_id);
    zend_update_property_string(FILE_IO_GLOBAL(http_server_class), this, PROP("serverAddress"),
                                host == NULL ? "0.0.0.0" : host);
    zend_update_property_long(FILE_IO_GLOBAL(http_server_class), this, PROP("serverPort"), port);
    uv_poll_t *handle = emalloc(sizeof(uv_poll_t));  //TODO FREE on server shutdown
//    context = php_stream_context_from_zval(NULL, flags & PHP_FILE_NO_DEFAULT_CONTEXT);
//    if (context) {
//        GC_ADDREF(context->res);
//    }
    LOG("Server id is %ld\n", Z_LVAL(id));
    http_php_servers[cur_id].server_stream = _php_stream_xport_create(full_host, ret_sz, REPORT_ERRORS,
                                                                      STREAM_XPORT_SERVER | (int) flags,

                                                                      NULL, NULL, NULL, &errstr, &err);

    if (http_php_servers[cur_id].server_stream == NULL) {
        php_error_docref(NULL, E_WARNING, "Unable to connect  %s\n",
                         errstr == NULL ? "Unknown error" : ZSTR_VAL(errstr));
    }

    LOG("Stream errors %d\n", err);
    int ret = set_non_blocking(http_php_servers[cur_id].server_stream);
    LOG("Set non block result: %d\n", ret);
    zend_result cast_result;
    http_php_servers[cur_id].server_fd = cast_to_fd(http_php_servers[cur_id].server_stream, &cast_result);
    LOG("Server FD is: %d\n", http_php_servers[cur_id].server_fd);
    get_meta_data(http_php_servers[cur_id].server_stream);

    init_cb(&fci, &fcc, &http_php_servers[cur_id].on_connect);

    if (SUCCESS == cast_result && ret == 1 && http_php_servers[cur_id].server_fd != -1) {
        uv_poll_init_socket(FILE_IO_GLOBAL(loop), handle, http_php_servers[cur_id].server_fd);
//        uv_signal_t *sig_handle = emalloc(sizeof(uv_signal_t)); //TODO FREE on server shutdown
//        uv_signal_init(FILE_IO_GLOBAL(loop), sig_handle);
//        uv_cb_type uv = {};
//        LOG("size of timeout handler %lu, fci  %lu \n\n", sizeof *idle_type, sizeof *fci);
        handle->data = (event_handle_item *) emalloc(sizeof(event_handle_item)); //TODO FREE on server shutdown
        http_php_servers[cur_id].clients_count = 0;
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


static PHP_FUNCTION (server_on_request) {
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    zend_fcall_info fci = empty_fcall_info;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_FUNC(fci, fcc);ZEND_PARSE_PARAMETERS_END();
    GET_HTTP_SERV_ID();
    LOG("Server_on_data  Server id is %lld\n", cur_id);
    init_cb(&fci, &fcc, &http_php_servers[cur_id].on_data);
//    http_php_servers[cur_id].on_data.fcc.object = http_php_servers[cur_id].on_data.fci.object = Z_OBJ_P(ZEND_THIS);
//    http_php_servers[cur_id].on_data.fcc.called_scope = Z_OBJCE_P(ZEND_THIS);
//    http_php_servers[cur_id].on_data.fcc.calling_scope = Z_OBJCE_P(ZEND_THIS);
    if (!ZEND_FCI_INITIALIZED(fci)) {
        zend_throw_error(NULL, "on data is not initialized");
    }
}

static PHP_FUNCTION (server_on_disconnect) {
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    zend_fcall_info fci = empty_fcall_info;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_FUNC(fci, fcc);ZEND_PARSE_PARAMETERS_END();
    GET_HTTP_SERV_ID();
    LOG("Server_on_disconnect  Server id is %ld\n", cur_id);
    init_cb(&fci, &fcc, &http_php_servers[cur_id].on_disconnect);
    if (!ZEND_FCI_INITIALIZED(fci)) {
        zend_throw_error(NULL, "on disconnect is not initialized");
    }
}

static PHP_FUNCTION (server_on_connect) {
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    zend_fcall_info fci = empty_fcall_info;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_FUNC(fci, fcc);ZEND_PARSE_PARAMETERS_END();
    GET_HTTP_SERV_ID();
    LOG("Server_on_connect  Server id is %ld\n", cur_id);
    init_cb(&fci, &fcc, &http_php_servers[cur_id].on_connect);
    if (!ZEND_FCI_INITIALIZED(fci)) {
        zend_throw_error(NULL, "on disconnect is not initialized");
    }
}

static PHP_FUNCTION (server_on_error) {
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    zend_fcall_info fci = empty_fcall_info;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_FUNC(fci, fcc);ZEND_PARSE_PARAMETERS_END();
    GET_HTTP_SERV_ID();
    LOG("Server_on_error  Server id is %ld\n", cur_id);
    init_cb(&fci, &fcc, &http_php_servers[cur_id].on_error);
    if (!ZEND_FCI_INITIALIZED(fci)) {
        zend_throw_error(NULL, "on error is not initialized");
    }
}

ZEND_METHOD (HttpServer, setReadBufferSize) {
    zend_long size = 1024;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_LONG(size);ZEND_PARSE_PARAMETERS_END();

    zend_update_property_long(Z_OBJ_P(ZEND_THIS)->ce, Z_OBJ_P(ZEND_THIS), PROP("readBufferSize"), size);

}


static void on_listen_server_for_clients(uv_poll_t *handle, int status, int events) {
    GET_HTTP_SERV_ID_FROM_EVENT_HANDLE();
    parse_uv_event(events, status);
    zend_string * errstr = NULL;
    zend_string * textaddr = NULL;
    php_stream *clistream = NULL;


    php_stream_xport_accept(http_php_servers[cur_id].server_stream, &clistream, &textaddr, NULL, NULL, NULL, &errstr);

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
        http_php_servers[cur_id].clients_count++;
        http_client_type *que_cli_handle = emalloc(sizeof(http_client_type));
        memset(que_cli_handle, 0, sizeof(http_client_type));
        que_cli_handle->current_stream = clistream;
        clistream = NULL;
        uv_timer_t * timer_h = emalloc(sizeof(uv_timer_t));
        uv_timer_init(FILE_IO_GLOBAL(loop), timer_h);

        que_cli_handle->current_fd = this_fd;
        que_cli_handle->close_timer = timer_h;
        que_cli_handle->client_handle = cli_handle;
        this_fd = -1;
        unsigned long long id = add_http_client_stream_handle(http_php_servers[cur_id].http_client_stream_handle_map,
                                                              que_cli_handle);
        uv_poll_init_socket(FILE_IO_GLOBAL(loop), cli_handle, que_cli_handle->current_fd); // if the same what to do?
//        event_handle_item handleItem = {.cur_id=cur_id, .handle_data=(void *) id, .this=((event_handle_item *) handle->data)->this};
        event_handle_item *handleItem = emalloc(sizeof(event_handle_item));
        request_info *requestInfo = emalloc(sizeof(request_info));
        requestInfo->id = id;
        requestInfo->is_read = false;
        timer_h->data = requestInfo;
        handleItem->cur_id = cur_id;
        handleItem->handle_data = requestInfo;
        handleItem->this = ((event_handle_item *) handle->data)->this;
        cli_handle->data = handleItem;
        zend_update_property_string(FILE_IO_GLOBAL(http_server_class), handleItem->this, PROP("clientAddress"),
                                    textaddr);
        uv_poll_start(cli_handle, UV_READABLE | UV_DISCONNECT | UV_WRITABLE, on_listen_client_event);
    } else {
        php_error_docref(NULL, E_ERROR, "Accept failed: %s", errstr ? ZSTR_VAL(errstr) : "Unknown error");
    }
    LOG("Client counts  by handles %d by counter %llu\n",
        count_http_client_stream_handles,
        http_php_servers[cur_id].clients_count);
//    exit(0);
    zend_long error = 0;
    zval retval;
    zval obj[1];
    ZVAL_OBJ(&obj[0], ((event_handle_item *) handle->data)->this);
    http_php_servers[cur_id].on_connect.fci.params = obj;
    http_php_servers[cur_id].on_connect.fci.param_count = 1;
    http_php_servers[cur_id].on_connect.fci.object = ((event_handle_item *) handle->data)->this;
    http_php_servers[cur_id].on_connect.fcc.object = ((event_handle_item *) handle->data)->this;
    http_php_servers[cur_id].on_connect.fcc.called_scope = ((event_handle_item *) handle->data)->this->ce;
    http_php_servers[cur_id].on_connect.fcc.calling_scope = ((event_handle_item *) handle->data)->this->ce;
    http_php_servers[cur_id].on_connect.fci.retval = &retval;
    //    zend_call_method_with_1_params(NULL, NULL, NULL, "print_r", &retval, &dstr);
    if (ZEND_FCI_INITIALIZED(http_php_servers[cur_id].on_connect.fci)) {
        if (zend_call_function(&http_php_servers[cur_id].on_connect.fci, &http_php_servers[cur_id].on_connect.fcc) !=
            SUCCESS) {
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

 void close_timer_cb(uv_timer_t* timer_h){
     LOG("Timer fires - setting is_read");
    request_info * requestInfo = timer_h->data;
    requestInfo->is_read = true;
    timer_h->data=NULL;
    efree(timer_h);
}


static void on_listen_client_event(uv_poll_t *handle, int status, int events) {
    GET_HTTP_SERV_ID_FROM_EVENT_HANDLE();
    parse_uv_event(events, status);
    event_handle_item *event_handle = (event_handle_item *) handle->data;
    request_info *requestInfo = (request_info *) event_handle->handle_data;
    unsigned long long id = requestInfo->id;
    http_client_stream_id_item_t *client = find_http_client_stream_handle(
            http_php_servers[cur_id].http_client_stream_handle_map, id);
    php_stream *clistream = client->handle->current_stream;

    zend_long error = 0;
    zval * closable_zv;
    long closable;
    closable_zv = zend_read_property(event_handle->this->ce, event_handle->this, CLOSABLE, sizeof(CLOSABLE) - 1, 0,
                                     NULL);
    closable = Z_TYPE_INFO_P(closable_zv);
    if (events & UV_WRITABLE) {
        on_ready_to_write(handle,client, status, events);
    }

    on_ready_to_disconnect(handle, client, status, events);

    if (events & UV_READABLE && clistream) {
        printf("on readable clistream %d unread %lld is_read %d ", clistream == NULL,
               clistream->writepos - clistream->readpos, requestInfo->is_read);
        uv_timer_t *timer_h = client->handle->close_timer;
        if (timer_h != NULL)
            uv_timer_stop(timer_h);
        zend_string * body = NULL;
        zend_string * headers = NULL;
        zval retval;
        zval args[2];
        LOG("Unread bytes before read %lld\n", clistream->writepos - clistream->readpos);
//        get_meta_data(clistream);
        /// get headers////
        char len[] = "\r\n\r\n";
        zval reqObj;
        zval resObj;
        if ((headers = php_stream_get_record(clistream, PHP_SOCK_CHUNK_SIZE, len, strlen(len)))) {
            zend_object * request;

            object_init_ex(&reqObj, FILE_IO_GLOBAL(http_request_class));
            request = Z_OBJ(reqObj);
            parse(ZSTR_VAL(headers), ZSTR_LEN(headers), request);

            ZVAL_OBJ(&reqObj, request);
//            php_var_dump(&reqObj, 1);
            LOG("Unread bytes after read  %lld\n", clistream->writepos - clistream->readpos);
            zend_off_t data_buf = clistream->writepos - clistream->readpos;
            /// get headers////
            body = php_stream_read_to_str(clistream, data_buf); //TODO RESTORE SIZE TO READ OR USE ANOTHER READ FUCNTION
            int position = php_stream_tell(clistream);
//            get_meta_data(clistream);
            zend_update_property_string(request->ce, request, PROP("body"), ZSTR_VAL(body));
            zval server;
            ZVAL_OBJ(&server, event_handle->this);
            LOG("Data pointer pos -  %d, Server id is %ld\n", position, cur_id);
            object_init_ex(&resObj, FILE_IO_GLOBAL(http_response_class));
            zend_update_property(Z_OBJ(resObj)->ce, Z_OBJ(resObj), PROP("server"), &server);
            zend_update_property_long(Z_OBJ(resObj)->ce, Z_OBJ(resObj), PROP("current_cli"), id);
//            request = Z_OBJ(resObj);
            ZVAL_COPY(&args[0], &reqObj);
            ZVAL_COPY(&args[1], &resObj);
            http_php_servers[cur_id].on_data.fci.param_count = 2;
            http_php_servers[cur_id].on_data.fci.params = args;
            http_php_servers[cur_id].on_data.fci.retval = &retval;
            if (ZEND_FCI_INITIALIZED(http_php_servers[cur_id].on_data.fci)) {
                if (zend_call_function(&http_php_servers[cur_id].on_data.fci, &http_php_servers[cur_id].on_data.fcc) !=
                    SUCCESS) {
                    error = -1;
                }
            } else {
                error = -2;
            }
            uv_timer_start(timer_h, close_timer_cb, 1, 0);
            requestInfo->is_read = true;
            parse_fci_error(error, "on data");
        }
    } else {
//        puts("no headers are present");
        //do something of no headers
    }

}


static void on_ready_to_write(uv_poll_t *handle,  http_client_stream_id_item_t * client, int status, int events) {
    event_handle_item *event_handle = (event_handle_item *) handle->data;
    php_stream *clistream = client->handle->current_stream;
    if (clistream != NULL) {
        if (client->handle->write_buf.len > 1) {
            LOG("**Data get from buffer TO CLIENT %lld:  sz:%zu**\n", client->handle_id,
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

static void on_ready_to_disconnect(uv_poll_t *handle,  http_client_stream_id_item_t * client, int status, int events) {
    GET_HTTP_SERV_ID_FROM_EVENT_HANDLE();
    php_stream *clistream = client->handle->current_stream;
    event_handle_item *event_handle = (event_handle_item *) handle->data;
    request_info *requestInfo = (request_info *) event_handle->handle_data;
    zend_long error = 0;
    if (events & UV_DISCONNECT)
        LOG("-------------- READY FOR DISCONNECT(%d) clients %d----------\n", events, count_http_client_stream_handles);

    if (requestInfo->is_read && (php_stream_eof(clistream) || (clistream->writepos - clistream->readpos) >= 0) &&
        client->handle->write_buf.len <= 1) {
//zend_fcall_info_argn
        uv_poll_stop(handle);
        if (events == 5 && ZEND_FCI_INITIALIZED(http_php_servers[cur_id].on_disconnect.fci)) {
            if (zend_call_function(&http_php_servers[cur_id].on_disconnect.fci,
                                   &http_php_servers[cur_id].on_disconnect.fcc) !=
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
        remove_http_client_stream_handle(http_php_servers[cur_id].http_client_stream_handle_map, client->handle_id);
        efree(client->handle);
        efree(handle->data);
        efree(handle);
        http_php_servers[cur_id].clients_count--;
        return;
    }
}

static const zend_function_entry class_HttpServer_methods[] = {
        ZEND_ME_MAPPING(__construct, server, arginfo_http_server, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(on_request, server_on_request, arginfo_http_server_event_handler, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(on_connect, server_on_connect, arginfo_http_server_event_handler, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(on_disconnect, server_on_disconnect, arginfo_http_server_event_handler, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(on_error, server_on_error, arginfo_http_server_event_handler, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

zend_class_entry *register_class_HttpServer(void) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "HttpServer", class_HttpServer_methods);
    FILE_IO_GLOBAL(http_server_class) = zend_register_internal_class_ex(&ce, NULL);
    FILE_IO_GLOBAL(http_server_class)->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES | ZEND_ACC_NOT_SERIALIZABLE;
    zend_declare_property_long(FILE_IO_GLOBAL(http_server_class), PROP(SERVER_ID), server_id,
                               ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_bool(FILE_IO_GLOBAL(http_server_class), PROP(CLOSABLE), 1,
                               ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_string(FILE_IO_GLOBAL(http_server_class), PROP("serverAddress"), "",
                                 ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_long(FILE_IO_GLOBAL(http_server_class), PROP("serverPort"), 0,
                               ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_long(FILE_IO_GLOBAL(http_server_class), PROP("readBufferSize"), 1024,
                               ZEND_ACC_PUBLIC);
    zend_declare_property_string(FILE_IO_GLOBAL(http_server_class), PROP("clientAddress"), "",
                                 ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_string(FILE_IO_GLOBAL(http_server_class), PROP("clientAddress2"), "",
                                 ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);

    return FILE_IO_GLOBAL(http_server_class);
}

static const zend_function_entry class_ServerRequest_methods[] = {
        PHP_FE_END
};


zend_class_entry *register_class_HttpRequest(void) {
    zend_class_entry ce;
    short flags_pb_ro = ZEND_ACC_PUBLIC | ZEND_ACC_READONLY;
    INIT_CLASS_ENTRY(ce, "HttpRequest", class_ServerRequest_methods);
    FILE_IO_GLOBAL(http_request_class) = zend_register_internal_class_ex(&ce, NULL);
    FILE_IO_GLOBAL(http_request_class)->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
    zend_declare_property_string(FILE_IO_GLOBAL(http_request_class), PROP("method"), "", flags_pb_ro);
    zend_declare_property_string(FILE_IO_GLOBAL(http_request_class), PROP("HttpVersion"), "", flags_pb_ro);
    zend_declare_property_string(FILE_IO_GLOBAL(http_request_class), PROP("uri"), "", flags_pb_ro);
    zend_declare_property_string(FILE_IO_GLOBAL(http_request_class), PROP("querystring"), "", flags_pb_ro);
    zend_declare_property_string(FILE_IO_GLOBAL(http_request_class), PROP("body"), "", flags_pb_ro);
    zval ht;
    ZVAL_EMPTY_ARRAY(&ht);//TODO REWRITE ON TYPED PROPERTY
    zend_declare_property(FILE_IO_GLOBAL(http_request_class), PROP("headers"), &ht, flags_pb_ro);
    zval query;
    ZVAL_EMPTY_ARRAY(&query);//TODO REWRITE ON TYPED PROPERTY
    zend_declare_property(FILE_IO_GLOBAL(http_request_class), PROP("query"), &query, flags_pb_ro);

    return FILE_IO_GLOBAL(http_request_class);
}
