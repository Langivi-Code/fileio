//
// Created by admin on 09.02.2022.
//

#ifndef FILEIO_DB_POLYFILL_H
#define FILEIO_DB_POLYFILL_H
#include "mysqli/php_mysqli_structs.h"
#include "php.h"
#include <libpq-fe.h>
#include <libpq/libpq-fs.h>
#include "db_async_override.h"
#include "mysqlnd/mysqlnd.h"
#define mysqlnd_async_query(conn, query_str, query_len)    ((conn)->data)->m->send_query((conn)->data, (query_str), (query_len), MYSQLND_SEND_QUERY_EXPLICIT, NULL, NULL)
#define mysqlnd_reap_async_query(conn)                    ((conn)->data)->m->reap_query((conn)->data)
#define MYSQLI_STORE_RESULT 0
#define MYSQLI_USE_RESULT    1
#define Z_MYSQLI(zv) php_mysqli_fetch_object(Z_OBJ((zv)))
typedef struct pgsql_link_handle {
    PGconn *conn;
    zend_string *hash;
    HashTable *notices;
    bool persistent;
    zend_object std;
} pgsql_link_handle;

typedef struct {
    MYSQLND *mysql;
    zend_string *hash_key;
    zval li_read;
    php_stream *li_stream;
    unsigned int multi_query;
    bool persistent;
    int async_result_fetch_type;
} MY_MYSQLND;
typedef struct _php_pgsql_result_handle {
    PGconn *conn;
    PGresult *result;
    int row;
    zend_object std;
} pgsql_result_handle;

static inline pgsql_link_handle *pgsql_link_from_obj(zend_object *obj) {
    return (pgsql_link_handle *) ((char *) (obj) - XtOffsetOf(pgsql_link_handle, std));
}

static inline pgsql_result_handle *pgsql_result_from_obj(zend_object *obj) {
    return (pgsql_result_handle *) ((char *) (obj) - XtOffsetOf(pgsql_result_handle, std));
}
#endif //FILEIO_DB_POLYFILL_H
