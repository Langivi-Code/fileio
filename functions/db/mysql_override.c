//
// Created by admin on 09.07.2022.
//
#include <php.h>
#include <uv.h>
#include <zend_API.h>
#include "db_async_override.h"
#include "mysqlnd/mysqlnd.h"
#include "../web/helpers.h"
#include "../common/call_php_fn.h"
#include "mysqlnd/mysqlnd_debug.h"
#include "mysqli/php_mysqli_structs.h"
#include "mysqli_priv.h"
#include "db_polyfill.h"
#include "../common/struct.h"
#include "../../php_fileio.h"
#include "../http/request.h"
#include "../common/find_class.h"


void my_poll_cb(uv_poll_t *handle, int status, int event) {
//    printf("event name %d ", event);
//    puts("polled");
    zval retval, retval_write;
    zval arg_read[1] = {0};
    zval arg_write[1] = {0};


    db_type_t *db_data = handle->data;
    zend_object * obj = Z_OBJ_P(db_data->db_handle);

    if (event & UV_WRITABLE && !db_data->written)    //WRITE TO FD
    {
        puts("Writable");
        zend_result pg_sent = FAILURE;


        switch (db_data->type) {
            case MYSQL_DB:
                ZVAL_COPY(&arg_write[0], db_data->db_handle);
                break;
        }
        call_php_fn(&db_data->cb, 1, arg_write, &retval_write, "db_write_wait");
        if (Z_TYPE(retval_write) == IS_STRING) {
            switch (db_data->type) {
                case MYSQL_DB:
                    //ZVAL_COPY(&arg_write[0], db_data->db_handle); //RUN send Query
                    break;
            }
        } else {
            zend_type_error("Return value should be a string");
        }

    } else if (event & UV_READABLE && db_data->written) {
        puts("Readable");
        uv_poll_stop(handle);
        efree(handle);
        zend_string * lcname;
        switch (db_data->type) {
            case MYSQL_DB:
                ZVAL_COPY(&arg_read[0], db_data->db_handle);

                MY_MYSQL *mysql;
                zval * mysql_link = db_data->db_handle;
                MYSQLI_RESOURCE *result_resource;
                MYSQL_RES *result = NULL;

                MYSQLI_RESOURCE *my_res;
                mysqli_object *intern = Z_MYSQLI_P(mysql_link);
                if (!(my_res = (MYSQLI_RESOURCE *) intern->ptr)) {
                    zend_throw_error(NULL, "%s object is already closed", ZSTR_VAL(intern->zo.ce->name));
                    ZEND_ASSERT(EG(exception));
                    break;
                }
                mysql = (MY_MYSQL *) my_res->ptr;
                if (my_res->status < MYSQLI_STATUS_VALID) {
                    zend_throw_error(NULL, "%s object is not fully initialized", ZSTR_VAL(intern->zo.ce->name));
                    ZEND_ASSERT(EG(exception));
                    break;
                }
                if (FAIL == mysqlnd_reap_async_query(mysql->mysql)) {
                    ZVAL_FALSE(&arg_read[0]);
                }
                if (!mysql_field_count(mysql->mysql)) {
                    /* no result set - not a SELECT */
                    ZVAL_TRUE(&arg_read[0]);
                    break;
                }

                switch (mysql->async_result_fetch_type) {
                    case MYSQLI_STORE_RESULT:
                        result = mysqlnd_store_result(mysql->mysql);
                        break;
                    case MYSQLI_USE_RESULT:
                        result = mysqlnd_use_result(mysql->mysql);
                        break;
                }
                result_resource = ecalloc(1, sizeof(MYSQLI_RESOURCE));
                result_resource->ptr = result;
                result_resource->status = MYSQLI_STATUS_VALID;

//                if (1) {
                    /* Ignore leading "\" */

                zend_class_entry *  mysqli_result_class_entry = find_class_by_name("mysqli_result");
//                } else {
//                    mysqli_result_class_entry = zend_lookup_class(name);
//                }

                ZVAL_OBJ(&arg_read[0], mysqli_objects_new(mysqli_result_class_entry));
                Z_MYSQLI(arg_read[0])->ptr = result_resource;
                break;
        }
        call_php_fn(&db_data->cb_read, 1, arg_read, &retval, "db_wait");
        efree(db_data);
    }
}



ZEND_FUNCTION(mysqli_wait) {
        //TODO TRY TO FETCH MYSQLI ISNTANSE
        MYSQLND *p = NULL;
        MY_MYSQL *obj;
        zval * db;
        zend_fcall_info fci = empty_fcall_info;
        zend_fcall_info_cache fcc = empty_fcall_info_cache;
        ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ZVAL(db)
        Z_PARAM_FUNC(fci, fcc)ZEND_PARSE_PARAMETERS_END();
        MYSQLI_FETCH_RESOURCE_CONN(obj, db, MYSQLI_STATUS_VALID);
        p = obj->mysql;
        php_stream *stream = p->data->vio->data->m.get_stream((p)->data->vio);
        //    DBG_INF_FMT("conn=%"
        //    PRIu64
        //    " stream=%p", p->data->thread_id, stream);
        zend_result res;
        set_non_blocking(stream);
        int fd = cast_to_fd(stream, &res);
        uv_poll_t *handle = emalloc(sizeof(uv_poll_t));
        db_type_t *db_type = emalloc(sizeof(db_type_t));
        printf("fd is %d\n", fd);
        init_cb(&fci, &fcc, &db_type->cb);
        db_type->type = MYSQL_DB;
        db_type->db_handle = db;
        uv_poll_init(MODULE_GL(loop), handle, fd);
        handle->data = db_type;
        uv_poll_start(handle, UV_READABLE, my_poll_cb);
}


ZEND_BEGIN_ARG_INFO(arginfo_promise_construct, 0)
                ZEND_ARG_TYPE_INFO(0, closure, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry class_Promise_methods[] = {
        //   ZEND_ME_MAPPING(get_result, mysqli_query, arginfo_promise_construct, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

void register_Async_Mysqli() {
    zend_class_entry ce;
    zend_class_entry * mysqli_link_class_entry = zend_hash_str_find_ptr(CG(class_table), PROP("mysqli"));
    zend_class_entry * mysqli_result_class_entrqy = zend_hash_str_find_ptr(CG(class_table), PROP("mysqli_result"));
    INIT_CLASS_ENTRY(ce, "mysqli_async", class_Promise_methods);
    MODULE_GL(mysqli_async_class) = zend_register_internal_class_ex(&ce, mysqli_link_class_entry);
}