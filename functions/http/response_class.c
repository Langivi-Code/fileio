//
// Created by admin on 15.11.2021.
//
#include <php.h>
#include "../common/mem.h"
#include "response_class.h"
#include "response.h"
#include "../../php_fileio.h"
#include "../web/http_server_args_info.h"
#include "../web/http_server.h"
#include "../web/header.h"
#include "zend_smart_string.h"
#include "../../constants.h"

response_obj *responseObj_from_zend_obj(zend_object *obj) {
    return (response_obj *) ((char *) (obj) - XtOffsetOf(response_obj, std));
}

PHP_FUNCTION (response_write) {
    char *data;
    size_t data_len;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STRING(data, data_len)
    ZEND_PARSE_PARAMETERS_END();
    response_obj *responseObj;
    GET_HTTP_SERV_ID_FROM_RES(responseObj);
    zend_object *this = Z_OBJ_P(ZEND_THIS);
    LOG("response sent %d", responseObj->sent);
    printf("response current_client %llu\n", responseObj->current_client);
    unsigned long long current_client = responseObj->current_client;
    LOG("clID: %lld %lld\n", current_client, cur_id);

    if (responseObj->sent != 1 && current_client != FAILURE){
        zval *status_code_zv = zend_read_property(this->ce, this, PROP("statusCode"), 0, NULL);
        char message[100] = "\0";
        sprintf(message, "HTTP/1.1 %s\r\n", status_message(Z_LVAL_P(status_code_zv)));
        smart_string all_headers_with_data ={0};
        smart_string_appends(&all_headers_with_data, message);
        smart_string *headers = stringify(responseObj->headers);
        smart_string_appendl(&all_headers_with_data, headers->c, headers->len);
        smart_string_appendl(&all_headers_with_data, "\r\n", 2);
        smart_string_appendl(&all_headers_with_data, data, data_len);

        http_client_stream_id_item_t *client = find_http_client_stream_handle(
                http_php_servers[cur_id].http_client_stream_handle_map,
                current_client);

        zend_long len = all_headers_with_data.len;
        if (client->handle->write_buf.len == 0) {
            client->handle->write_buf = uv_buf_init(emalloc(sizeof(char) * len), len);
            memset(client->handle->write_buf.base, 0, len);
        } else {
            client->handle->write_buf.base = erealloc(client->handle->write_buf.base,
                                                      sizeof(char) * (client->handle->write_buf.len + len));
            client->handle->write_buf.len = client->handle->write_buf.len + len;
        }
        // @tips memcpy for copy of binary data
        memcpy(client->handle->write_buf.base, all_headers_with_data.c, len);

        LOG("Data(%zu) set to buffer", all_headers_with_data.len);

        responseObj->sent = true;
        LOG("Response sent after %d", responseObj->sent);
        smart_string_free(headers);
        efree(headers);
        smart_string_free(&all_headers_with_data);
    } else {
        zend_throw_error(NULL, "Response is already sent");
    }

}


ZEND_METHOD(HttpResponse, setHeader) {
    char *key, *val;
    zend_long key_len, val_len;
    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_STRING(key, key_len)
            Z_PARAM_STRING(val, val_len)
    ZEND_PARSE_PARAMETERS_END();
    puts("s1");
    response_obj *responseObj = responseObj_from_zend_obj(Z_OBJ_P(ZEND_THIS));
//    puts("object got");
//    responseObj->headers = create_kv_collection();
    append_string_to_kv_collection(&responseObj->headers, key, val);
    GC_TRY_ADDREF(Z_OBJ_P(ZEND_THIS));
    RETURN_OBJ(Z_OBJ_P(ZEND_THIS));
//    responseObj = (response_obj *) zend_object_store_get_object(getThis());
}

ZEND_METHOD(HttpResponse, setStatusCode) {
    zend_long code;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_LONG(code)
    ZEND_PARSE_PARAMETERS_END();
    zend_object *this = Z_OBJ_P(ZEND_THIS);
    zend_update_property_long(this->ce, this, PROP("statusCode"), code);
    RETURN_BOOL(1);
}


PHP_FUNCTION (response_end) {
    char *data;
    zend_long data_len;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STRING(data, data_len)
    ZEND_PARSE_PARAMETERS_END();
    response_obj *responseObj;
    GET_HTTP_SERV_ID_FROM_RES(responseObj);
    unsigned long long current_client = responseObj->current_client;;
    if (current_client != FAILURE) {
        http_client_stream_id_item_t *client = find_http_client_stream_handle(
                http_php_servers[cur_id].http_client_stream_handle_map,
                current_client);

//        uv_poll_stop(client->handle->client_handle);
        php_stream_free(client->handle->current_stream, PHP_STREAM_FREE_KEEP_RSRC |
                                                        (client->handle->current_stream->is_persistent
                                                         ? PHP_STREAM_FREE_CLOSE_PERSISTENT
                                                         : PHP_STREAM_FREE_CLOSE));
//    http_php_servers[cur_id].current_client_stream = NULL;

        remove_http_client_stream_handle(http_php_servers[cur_id].http_client_stream_handle_map, current_client);
//        efree(client->handle->client_handle->data);
//        efree(client->handle->client_handle);
        efree(client->handle);
        http_php_servers[cur_id].clients_count--;
    }

}

static zend_object_handlers response_object_handlers;

zend_object *create_response_obj(zend_class_entry *class_type) {
    response_obj *internal_obj = zend_object_alloc(sizeof(response_obj), class_type);
    memset(internal_obj, 0, sizeof(response_obj));
//    zend_object_alloc()

    zend_object_std_init(&internal_obj->std, class_type);
    object_properties_init(&internal_obj->std, class_type);
    internal_obj->headers = create_kv_collection();
//    internal_obj->sent = false;
    internal_obj->std.handlers = &response_object_handlers;
    return &internal_obj->std;
}

void free_response_obj(void *object) {
    response_obj *res = (response_obj *) object;
    efree(res->headers.kv);
//    if (res->headers){
//        efree(res->headers);
//    }
    efree(res);
}

//static HashTable *response_obj_get_gc(zend_object *object, zval **table, int *n){
//
//}

int response_obj_cast_object(zend_object *obj, zval *result, int type) {
    if (type == IS_LONG) {
        /* For better backward compatibility, make (int) $curl_handle return the object ID,
         * similar to how it previously returned the resource ID. */
        ZVAL_LONG(result, obj->handle);
        return SUCCESS;
    }

    return zend_std_cast_object_tostring(obj, result, type);
}


static const zend_function_entry class_ServerResponse_methods[] = {
        ZEND_ME (HttpResponse, setStatusCode, arginfo_http_server_set_status_code, ZEND_ACC_PUBLIC)
        ZEND_ME (HttpResponse, setHeader, arginfo_http_server_set_header, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(send, response_write, arginfo_http_server_write, ZEND_ACC_PUBLIC)
        ZEND_ME_MAPPING(end, response_end, arginfo_http_server_write, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

zend_class_entry *register_class_HttpResponse(void) {
    zend_class_entry ce;
    short flags_pb_ro = ZEND_ACC_PUBLIC | ZEND_ACC_READONLY;
    INIT_CLASS_ENTRY(ce, "HttpResponse", class_ServerResponse_methods);
    ce.create_object = create_response_obj;
    memcpy(&response_object_handlers, &std_object_handlers, sizeof(zend_object_handlers));
    response_object_handlers.offset = XtOffsetOf(response_obj, std);
    response_object_handlers.free_obj = free_response_obj;
    response_object_handlers.get_gc = NULL;
//    response_object_handlers.get_constructor = curl_multi_get_constructor;
    response_object_handlers.clone_obj = NULL;
    response_object_handlers.cast_object = response_obj_cast_object;
    response_object_handlers.compare = zend_objects_not_comparable;


    FILE_IO_GLOBAL(http_response_class) = zend_register_internal_class_ex(&ce, NULL);
//    FILE_IO_GLOBAL(http_request_class)->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES | ZEND_ACC_NOT_SERIALIZABLE;
//    zval property_statusCode_default_value;
//    ZVAL_UNDEF(&property_statusCode_default_value);
//    zend_string *property_statusCode = zend_string_init(PROP("statusCode"), 1);
//    zend_declare_typed_property(FILE_IO_GLOBAL(http_response_class), property_statusCode, &property_statusCode_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
//    zend_string_release(property_statusCode);
    zend_declare_property_long(FILE_IO_GLOBAL(http_response_class), PROP("statusCode"), 200, flags_pb_ro);
    zend_declare_property_string(FILE_IO_GLOBAL(http_response_class), PROP("body"), "", flags_pb_ro);
    zval ht;
    ZVAL_EMPTY_ARRAY(&ht);//TODO REWRITE ON TYPED PROPERTY
    zend_declare_property(FILE_IO_GLOBAL(http_response_class), PROP("headers"), &ht, flags_pb_ro);


    return FILE_IO_GLOBAL(http_request_class);
}