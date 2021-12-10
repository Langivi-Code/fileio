//
// Created by user on 25.10.21.
//
#include <php.h>
#include <zend_API.h>

#include <uv.h>
#include <php_variables.h>
#include <json/php_json.h>
#include "../http/response_class.h"

#include "../common/fill_event_handle.h"
#include "../../php_fileio.h"
#include "ext/standard/file.h"
#include "ext/standard/php_var.h"
#include "http_server.h"
#include "helpers.h"
#include "../common/struct.h"
#include "http_server_args_info.h"
#include "header.h"
#include "../common/mem.h"

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
http_server_type http_php_servers[10];


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
    zval *zerrno = NULL, *zerrstr = NULL, *zcontext = NULL;
    ZEND_PARSE_PARAMETERS_START(1, 3)
            Z_PARAM_LONG(port)
            Z_PARAM_OPTIONAL
            Z_PARAM_STRING_OR_NULL(host, host_len)
            Z_PARAM_ARRAY_HT_OR_NULL(options)
    ZEND_PARSE_PARAMETERS_END();
    int err = 0;
    zend_long flags = STREAM_XPORT_BIND | STREAM_XPORT_LISTEN;
    zend_string *errstr = NULL;
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
    zend_object *this = Z_OBJ_P(ZEND_THIS);
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
//    get_meta_data(http_php_servers[cur_id].server_stream);

    init_cb(&fci, &fcc, &http_php_servers[cur_id].on_connect);

    if (SUCCESS == cast_result && ret == 1 && http_php_servers[cur_id].server_fd != -1) {
        uv_poll_init_socket(FILE_IO_GLOBAL(loop), handle, http_php_servers[cur_id].server_fd);
//        uv_signal_t *sig_handle = emalloc(sizeof(uv_signal_t)); //TODO FREE on server shutdown
//        uv_signal_init(FILE_IO_GLOBAL(loop), sig_handle);
//        uv_cb_type uv = {};
//        LOG("size of timeout handler %lu, fci  %lu \n\n", sizeof *idle_type, sizeof *fci);
        handle->data = emalloc(sizeof(ht_event_handle_item)); //TODO FREE on server shutdown
        ((ht_event_handle_item *) handle->data)->this = this;
        ((ht_event_handle_item *) handle->data)->cur_id = cur_id;
        http_php_servers[cur_id].clients_count = 0;

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
            Z_PARAM_FUNC(fci, fcc);
    ZEND_PARSE_PARAMETERS_END();
    GET_HTTP_SERV_ID();
    LOG("Server_on_data  Server id is %lld\n", cur_id);
    init_cb(&fci, &fcc, &http_php_servers[cur_id].on_data);

    http_php_servers[cur_id].on_data.fci.object = Z_OBJ_P(ZEND_THIS);
    http_php_servers[cur_id].on_data.fcc.object = Z_OBJ_P(ZEND_THIS);
    http_php_servers[cur_id].on_data.fcc.called_scope = Z_OBJCE_P(ZEND_THIS);
    http_php_servers[cur_id].on_data.fcc.calling_scope = Z_OBJCE_P(ZEND_THIS);
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
            Z_PARAM_FUNC(fci, fcc);
    ZEND_PARSE_PARAMETERS_END();
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
            Z_PARAM_FUNC(fci, fcc);
    ZEND_PARSE_PARAMETERS_END();
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
            Z_PARAM_FUNC(fci, fcc);
    ZEND_PARSE_PARAMETERS_END();
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
            Z_PARAM_LONG(size);
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_long(Z_OBJ_P(ZEND_THIS)->ce, Z_OBJ_P(ZEND_THIS), PROP("readBufferSize"), size);

}


static void on_listen_server_for_clients(uv_poll_t *handle, int status, int events) {
    GET_HTTP_SERV_ID_FROM_EVENT_HANDLE();
    parse_uv_event(events, status);
    zend_string *errstr = NULL;
    zend_string *textaddr = NULL;
    php_stream *clistream = NULL;


    php_stream_xport_accept(http_php_servers[cur_id].server_stream, &clistream, &textaddr, NULL, NULL, NULL, &errstr);

    if (!clistream) {
        php_error_docref(NULL, E_ERROR, "Accept failed: %s", errstr ? ZSTR_VAL(errstr) : "Unknown error");
    }
    zend_result cast_result;
    int this_fd = cast_to_fd(clistream, &cast_result);
    int ret = set_non_blocking(clistream);


    LOG("non block %d cast %d\n", ret, cast_result);
    LOG("Client address : %s\n", textaddr ? ZSTR_VAL(textaddr) : "no address");
    LOG("New connection accepted fd is %d\n ", this_fd);

    if (cast_result == SUCCESS && ret == SUCCESS) {
//        uv_timer_t * timer_h = emalloc(sizeof(uv_timer_t));
//        uv_timer_init(FILE_IO_GLOBAL(loop), timer_h);
        http_php_servers[cur_id].clients_count++;
        uv_poll_t *client_poll_handle;
        http_client_type *que_cli_handle;

        client_poll_handle = emalloc(sizeof(uv_poll_t));
        que_cli_handle = emalloc(sizeof(http_client_type));
        memset(que_cli_handle, 0, sizeof(http_client_type));
//        alloc_handles(client_poll_handle, que_cli_handle);

        que_cli_handle->current_stream = clistream;

        puts("hhh");
        que_cli_handle->current_fd = this_fd;
//        que_cli_handle->close_timer = timer_h;

        unsigned long long id = add_http_client_stream_handle(http_php_servers[cur_id].http_client_stream_handle_map,
                                                              que_cli_handle);
        uv_poll_init_socket(FILE_IO_GLOBAL(loop), client_poll_handle, que_cli_handle->current_fd); // if the same what to do?
//        ht_event_handle_item handleItem = {.cur_id=cur_id, .handle_data=(void *) id, .this=((ht_event_handle_item *) handle->data)->this};
        ht_event_handle_item *handleItem = emalloc(sizeof(ht_event_handle_item));
        add_pntr(&que_cli_handle->pointers, handleItem);
        handleItem->req_info.id = id;
        handleItem->req_info.is_read = false;
        handleItem->cur_id = cur_id;
        handleItem->this = ((ht_event_handle_item *) handle->data)->this;
        client_poll_handle->data = handleItem;

        zend_update_property_string(FILE_IO_GLOBAL(http_server_class), handleItem->this, PROP("clientAddress"),
                                    textaddr);
        uv_poll_start(client_poll_handle, UV_READABLE | UV_DISCONNECT | UV_WRITABLE, on_listen_client_event);
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
    ZVAL_OBJ(&obj[0], ((ht_event_handle_item *) handle->data)->this);
    http_php_servers[cur_id].on_connect.fci.params = obj;
    http_php_servers[cur_id].on_connect.fci.param_count = 1;
    http_php_servers[cur_id].on_connect.fci.object = ((ht_event_handle_item *) handle->data)->this;
    http_php_servers[cur_id].on_connect.fcc.object = ((ht_event_handle_item *) handle->data)->this;
    http_php_servers[cur_id].on_connect.fcc.called_scope = ((ht_event_handle_item *) handle->data)->this->ce;
    http_php_servers[cur_id].on_connect.fcc.calling_scope = ((ht_event_handle_item *) handle->data)->this->ce;
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

void close_timer_cb(uv_timer_t *timer_h) {
    LOG("Timer fires - setting is_read");
    request_info *requestInfo = timer_h->data;
    requestInfo->is_read = true;
    timer_h->data = NULL;
    efree(timer_h);
}


static void on_listen_client_event(uv_poll_t *handle, int status, int events) {
    GET_HTTP_SERV_ID_FROM_EVENT_HANDLE();
    parse_uv_event(events, status);
    ht_event_handle_item *event_handle = (ht_event_handle_item *) handle->data;
    unsigned long long id = event_handle->req_info.id;
    http_client_stream_id_item_t *client = find_http_client_stream_handle(
            http_php_servers[cur_id].http_client_stream_handle_map, id);
    php_stream *clistream = client->handle->current_stream;


    if (events & UV_WRITABLE) {
        on_ready_to_write(handle, client, status, events);
    }

    bool disconnected = on_ready_to_disconnect(handle, client, status, events);

    if (events & UV_READABLE && clistream != NULL && ! disconnected) {
        on_ready_to_read(handle, client, status, events);
    }

}

static void on_ready_to_read(uv_poll_t *handle, http_client_stream_id_item_t *client, int status, int events) {
    GET_HTTP_SERV_ID_FROM_EVENT_HANDLE();
    php_stream *clistream = client->handle->current_stream;
    ht_event_handle_item *event_handle = (ht_event_handle_item *) handle->data;
    unsigned long long id = event_handle->req_info.id;
    zend_long error = 0;

    printf("on readable clistream %d unread %lld is_read %d ", clistream == NULL,
           clistream->writepos - clistream->readpos, event_handle->req_info.is_read);
//        uv_timer_t *timer_h = client->handle->close_timer;
//        if (timer_h !=/**/ NULL)
//            uv_timer_stop(timer_h);
    zend_string * body = NULL;
    zend_string * headers = NULL;
    zend_object * request;
    zend_off_t data_buf = clistream->writepos - clistream->readpos;
    zval retval, reqObj, resObj;
    zval args[2];
    LOG("Unread bytes before read %ld\n", data_buf);

    const long long size = data_buf ? data_buf : PHP_SOCK_CHUNK_SIZE;

    char len[] = "\r\n\r\n";

    /// get headers////
    headers = php_stream_get_record(clistream, PHP_SOCK_CHUNK_SIZE, len, strlen(len));

    if (headers) {
        LOG("something strange happening******************");
        object_init_ex(&reqObj, FILE_IO_GLOBAL(http_request_class));
        request = Z_OBJ(reqObj);

        parse(ZSTR_VAL(headers), ZSTR_LEN(headers), request);

        zend_string_release(headers);
//        add_pntr(&que_cli_handle->pointers, requestInfo);
        data_buf = clistream->writepos - clistream->readpos;

        LOG("Unread bytes after read  %lld\n", data_buf);

    } else {
        php_error_docref(NULL, E_ERROR, "Unable to read headers %s", ZSTR_VAL(headers));
    }

    data_buf = clistream->writepos - clistream->readpos;
    body = php_stream_read_to_str(clistream, data_buf); //TODO RESTORE SIZE TO READ OR USE ANOTHER READ FUCNTION
    ZVAL_OBJ(&reqObj, request);
    zval * _get = zend_read_property(request->ce, request, PROP("query"), 0, NULL);
    fill_super_global(TRACK_VARS_GET, _get);
    /******************************/
    zval * _headers = zend_read_property(request->ce, request, PROP("headers"), 0, NULL);
    zval * content_type = zend_hash_str_find(Z_ARR_P(_headers), PROP("Content-Type"));
    if (ZSTR_LEN(body) > 0) {
        if (strcmp(Z_STRVAL_P(content_type), "application/x-www-form-urlencoded") == 0) {
            puts(ZSTR_VAL(body));
            puts("body found");
            key_value_collection post_query = parse_querystring(ZSTR_VAL(body));
            printf("POST size %zu", post_query.size);
            zval zv_post;
            if (post_query.size > 0) {
                array_init_size(&zv_post, post_query.size);
                for (int i = 0; i < post_query.size; ++i) {
                    printf("key %s\n", post_query.kv[i].key);
                    printf("value %s\n", post_query.kv[i].value);
                    php_register_variable_safe(post_query.kv[i].key, post_query.kv[i].value,
                                               strlen(post_query.kv[i].value),
                                               &PG(http_globals)[TRACK_VARS_POST]);
                }

//
                zend_update_property(request->ce, request, PROP("body"), &PG(http_globals)[TRACK_VARS_POST]);
            }
        } else if (strcmp(Z_STRVAL_P(content_type), "application/json") == 0) {
            zval ret;
            zend_long options = 0;
            options |= PHP_JSON_OBJECT_AS_ARRAY;
            if (ZSTR_LEN(body) > 0) {
                if (php_json_decode_ex(&ret, ZSTR_VAL(body), ZSTR_LEN(body), options, 512) == SUCCESS) {
                    fill_super_global(TRACK_VARS_POST, &ret);
                }
            }
            zend_update_property(request->ce, request, PROP("body"), &PG(http_globals)[TRACK_VARS_POST]);
        }
    }



    /******************************/
    int position = php_stream_tell(clistream);


    zval server;
    ZVAL_OBJ(&server, event_handle->this);
    LOG("Data pointer pos -  %d, Server id is %ld\n", position, cur_id);
    object_init_ex(&resObj, FILE_IO_GLOBAL(http_response_class));

    zend_update_property(Z_OBJ(resObj)->ce, Z_OBJ(resObj), PROP("server"), &server);
    responseObj_from_zend_obj(Z_OBJ(resObj))->current_client = id;
    responseObj_from_zend_obj(Z_OBJ(resObj))->sent = false;

    ZVAL_COPY(&args[0], &reqObj);
    ZVAL_COPY(&args[1], &resObj);

#define print_ref(OBJECT) printf("ref %u " #OBJECT " \n", GC_REFCOUNT(OBJECT));

#define print_ref_str(STR) printf("ref %u " #STR " \n", zend_gc_refcount(&STR->gc));

    print_ref(Z_OBJ(retval));
    print_ref(event_handle->this);

    print_ref(Z_OBJ(resObj));
    print_ref(Z_OBJ(reqObj));
    print_ref(Z_OBJ(args[1]));
    print_ref(Z_OBJ(args[0]));
//    print_ref_str(body);
    print_ref_str(headers);
    print_ref(request);
//    print_ref(responseObj_from_zend_obj(Z_OBJ(resObj))->server);
    print_ref(Z_OBJ(server));

    http_php_servers[cur_id].on_data.fci.param_count = 2;
    http_php_servers[cur_id].on_data.fci.params = args;
    http_php_servers[cur_id].on_data.fci.retval = &retval;


    if (ZEND_FCI_INITIALIZED(http_php_servers[cur_id].on_data.fci)) {
        if (zend_call_function(&http_php_servers[cur_id].on_data.fci, &http_php_servers[cur_id].on_data.fcc) !=
            SUCCESS) {
            error = -1;
        } else {
//            GC_TRY_DELREF(event_handle->this);
        }
    } else {
        error = -2;
    }

//            uv_timer_start(timer_h, close_timer_cb, 1, 0);
    event_handle->req_info.is_read = true;
    parse_fci_error(error, "on data");


}

static void on_ready_to_write(uv_poll_t *handle, http_client_stream_id_item_t *client, int status, int events) {
    php_stream *clistream = client->handle->current_stream;
    if (clistream != NULL) {
        if (client->handle->write_buf.len > 1) {
            LOG("**Data get from buffer TO CLIENT %lld:  sz:%zu**\n", client->handle_id,
                client->handle->write_buf.len);
            php_stream_write(clistream, client->handle->write_buf.base,
                             client->handle->write_buf.len);
            efree(client->handle->write_buf.base);
            client->handle->write_buf.len = 0;
        }
    } else {
        puts("Client handle is empty");
    }

}

static bool on_ready_to_disconnect(uv_poll_t *handle, http_client_stream_id_item_t *client, int status, int events) {
    GET_HTTP_SERV_ID_FROM_EVENT_HANDLE();
    php_stream *clistream = client->handle->current_stream;
    ht_event_handle_item *event_handle = (ht_event_handle_item *) handle->data;

    zend_long error = 0;

    if (events & UV_DISCONNECT) {

        LOG("-------------- READY FOR DISCONNECT(%d) clients %d----------\n", events, count_http_client_stream_handles);
        uv_poll_stop(handle);
        php_stream_free(clistream, PHP_STREAM_FREE_KEEP_RSRC |
                                   (clistream->is_persistent ? PHP_STREAM_FREE_CLOSE_PERSISTENT
                                                             : PHP_STREAM_FREE_CLOSE));
        remove_http_client_stream_handle(http_php_servers[cur_id].http_client_stream_handle_map, client->handle_id);

        efree(event_handle);
        efree(handle);
        http_php_servers[cur_id].clients_count--;
//        zval_ptr_dtor(client->handle->request_object);
//        zval_ptr_dtor(client->handle->response_object);
//        efree(client->handle->write_buf.base);

        efree(client->handle);

        return true;
    }


    if (event_handle->req_info.is_read && (php_stream_eof(clistream) || (clistream->writepos - clistream->readpos) >= 0) &&
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

        return true;
    }
    return false;
}

ZEND_METHOD(HttpServer, setPublicPath){
    char * public_path;
    size_t public_path_len;
    ZEND_PARSE_PARAMETERS_START(1,1)
            Z_PARAM_STRING(public_path, public_path_len)
    ZEND_PARSE_PARAMETERS_END();
    zend_update_property_string(Z_OBJCE_P(ZEND_THIS), Z_OBJ_P(ZEND_THIS), PROP("publicPath"), public_path);
    RETURN_TRUE;
}


void add_pntr(pntrs_to_free *pointers_store, void *pointer) {
    if (pointers_store->size == 0) {
        pointers_store->pointers = emalloc(sizeof(void *));
        pointers_store->pointers[0] = pointer;
        pointers_store->size++;
    } else {
        pointers_store->pointers = erealloc(pointers_store->pointers, sizeof(void *) * (pointers_store->size + 1));
        pointers_store->pointers[pointers_store->size] = pointer;
        pointers_store->size++;
    }
}

static const zend_function_entry class_HttpServer_methods[] = {
        ZEND_ME_MAPPING(__construct, server, arginfo_http_server, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(on_request, server_on_request, arginfo_http_server_event_handler, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(on_connect, server_on_connect, arginfo_http_server_event_handler, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(on_disconnect, server_on_disconnect, arginfo_http_server_event_handler, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(on_error, server_on_error, arginfo_http_server_event_handler, ZEND_ACC_PUBLIC)
        ZEND_ME(HttpServer, setPublicPath, arginfo_http_server_set_public_path, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

zend_class_entry *register_class_HttpServer(void) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "HttpServer", class_HttpServer_methods);
    FILE_IO_GLOBAL(http_server_class) = zend_register_internal_class_ex(&ce, NULL);
    FILE_IO_GLOBAL(http_server_class)->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES | ZEND_ACC_NOT_SERIALIZABLE;
    zend_declare_property_long(FILE_IO_GLOBAL(http_server_class), PROP(SERVER_ID), server_id,
                               ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_string(FILE_IO_GLOBAL(http_server_class), PROP("serverAddress"), "",
                                 ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_long(FILE_IO_GLOBAL(http_server_class), PROP("serverPort"), 0,
                               ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_string(FILE_IO_GLOBAL(http_server_class), PROP("clientAddress"), "",
                                 ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_string(FILE_IO_GLOBAL(http_server_class), PROP("publicPath"), "",
                                 ZEND_ACC_PUBLIC);

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
