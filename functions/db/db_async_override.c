//
// Created by admin on 13.12.2021.
//
#include <php.h>
#include <uv.h>
#include <zend_API.h>
#include "db_async_override.h"
#include "mysqli/php_mysqli_structs.h"
#include "mysqlnd/mysqlnd.h"
#include "../web/helpers.h"
#include "../../php_fileio.h"
#include "mysqlnd/mysqlnd_debug.h"
#include "../http/request.h"
#include <libpq-fe.h>
#include <libpq/libpq-fs.h>
#include <pcap/socket.h>

typedef struct pgsql_link_handle {
    PGconn *conn;
    zend_string *hash;
    HashTable *notices;
    bool persistent;
    zend_object std;
} pgsql_link_handle;
static inline pgsql_link_handle *pgsql_link_from_obj(zend_object *obj) {
    return (pgsql_link_handle *)((char *)(obj) - XtOffsetOf(pgsql_link_handle, std));
}

void poll_cb(uv_poll_t *handle, int event, int status) {
    puts("Readable");
    uv_poll_stop(handle);
//    MYSQLND *conn = handle->data;
//    MYSQLND_RES *result = NULL;
//    zval * return_value;
//    int async_result_fetch_type = MYSQLI_STORE_RESULT;
////  MYSQLND_RES->data)->m->reap_query((conn)->data)
//    if (FAIL == mysqlnd_reap_async_query(conn)) {
////        if ((MyG(report_mode) & MYSQLI_REPORT_ERROR) && mysqlnd_errno(conn)) {
////
////            php_mysqli_report_error(mysqlnd_sqlstate(conn), mysqlnd_errno(conn), mysqlnd_error(conn));
////
////        }
//        RETURN_FALSE;
//    }
//
//    if (!mysqlnd_field_count(conn)) {
//        /* no result set - not a SELECT */
////        if (MyG(report_mode) & MYSQLI_REPORT_INDEX) {
/////*			php_mysqli_report_index("n/a", mysqli_server_status(mysql->mysql)); */
////        }
//        RETURN_TRUE;
//    }
//
//    switch (async_result_fetch_type) {
//        case MYSQLI_STORE_RESULT:
//            result = mysqlnd_store_result(conn);
//            break;
//        case MYSQLI_USE_RESULT:
//            result = mysqlnd_use_result(conn);
//            break;
//    }
//
//    if (!result) {
////        if ((MyG(report_mode) & MYSQLI_REPORT_ERROR) && mysqlnd_errno(conn)) {
////            php_mysqli_report_error(mysqlnd_sqlstate(conn), mysqlnd_errno(conn), mysqlnd_error(conn));
////        }
//        RETURN_FALSE;
//    }
//
////    if (MyG(report_mode) & MYSQLI_REPORT_INDEX) {
/////*		php_mysqli_report_index("n/a", mysqli_server_status(mysql->mysql)); */
////    }
//    MYSQLI_RESOURCE *mysqli_resource = (MYSQLI_RESOURCE *) ecalloc (1, sizeof(MYSQLI_RESOURCE));
//    mysqli_resource->ptr = (void *) result;
//    mysqli_resource->status = MYSQLI_STATUS_VALID;
//    MYSQLI_RETVAL_RESOURCE(mysqli_resource, mysqli_result_class_entry);

}

ZEND_FUNCTION(mysqli_wait) {
    //TODO TRY TO FETCH MYSQLI ISNTANSE
    MYSQLND *p = NULL;
    MY_MYSQL  *obj;
    zval *db;
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(db)
            Z_PARAM_FUNC(fci, fcc)
            ZEND_PARSE_PARAMETERS_END();
    MYSQLI_FETCH_RESOURCE_CONN(obj, db, MYSQLI_STATUS_VALID);
    p = obj->mysql;
    php_stream *stream = p->data->vio->data->m.get_stream((p)->data->vio);
//    DBG_INF_FMT("conn=%"
//    PRIu64
//    " stream=%p", p->data->thread_id, stream);
    zend_result res;
    set_non_blocking(stream);
    int fd = cast_to_fd(stream, &res);
    uv_poll_t * handle = emalloc(sizeof(uv_poll_t));
    printf("fd is %d\n", fd);
    uv_poll_init(MODULE_GL(loop), handle, fd);
    handle->data = p;
    uv_poll_start(handle, UV_READABLE, poll_cb);
}

ZEND_FUNCTION(pg_wait) {
    zend_result pg_exists = FAILURE;
    zend_string *module_name = zend_string_init(PROP("pgsql"),0);
    if (zend_hash_exists(&module_registry, module_name)) {
        zval *db;
        zend_fcall_info fci = empty_fcall_info;
        zend_fcall_info_cache fcc = empty_fcall_info_cache;
        pgsql_link_handle *link;
        ZEND_PARSE_PARAMETERS_START(2, 2)
                Z_PARAM_ZVAL(db)
                Z_PARAM_FUNC(fci, fcc)
        ZEND_PARSE_PARAMETERS_END();
        PGconn *pgsql;
        link = pgsql_link_from_obj(Z_OBJ_P(db));
        if (link->conn == NULL) {
		zend_throw_error(NULL, "PostgreSQL connection has already been closed");
		RETURN_THROWS();
	}

        pgsql = link->conn;
        int fd_number = PQsocket(pgsql);
        if (fd_number == -1) {
            zend_throw_error(NULL, "Can't allocate connectionsocket");
            RETURN_THROWS();
        }
        PQsetnonblocking(pgsql, 1);
        uv_poll_t * handle = emalloc(sizeof(uv_poll_t));
        printf("fd is %d\n", fd_number);
        uv_poll_init(MODULE_GL(loop), handle, fd_number);
        handle->data = pgsql;
        uv_poll_start(handle, UV_READABLE, poll_cb);
    } else{
        zend_throw_error(NULL, "PgSQL is not loaded\n");
        RETURN_THROWS();
    }
    zend_string_release(module_name);

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
    zend_class_entry *  mysqli_link_class_entry = zend_hash_str_find_ptr(CG(class_table), PROP("mysqli"));
    zend_class_entry * mysqli_result_class_entrqy = zend_hash_str_find_ptr(CG(class_table), PROP("mysqli_result"));
    INIT_CLASS_ENTRY(ce, "mysqli_async", class_Promise_methods);
    MODULE_GL(mysqli_async_class) = zend_register_internal_class_ex(&ce, mysqli_link_class_entry);
}