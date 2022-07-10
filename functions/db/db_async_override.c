//
// Created by admin on 13.12.2021.
//
#ifndef MYSQLI_USE_MYSQLND
#define MYSQLI_USE_MYSQLND
#endif
#define PG_PARALLEL

#include <php.h>
#include <uv.h>
#include <zend_API.h>
#include "db_async_override.h"
#include "../web/helpers.h"
#include "../../php_fileio.h"
#include "../http/request.h"
#include "../common/call_php_fn.h"
#include "../common/struct.h"
#include "../../rustlib/include/stdasync_lib.h"
#include "db_polyfill.h"
#include "standard/php_standard.h"
#include "../common/find_class.h"
#include <libpq-fe.h>
#include <libpq/libpq-fs.h>
#include <zend_exceptions.h>

enum function_name {
    QUERY,
    PREPARE,
    RUN_PREPARED
};

struct fd {
    uint16_t fd;
    enum function_name name;
};

static int query_poll_cb (int event, uint16_t fd){
     zval retval, retval_write;
     zval arg_read[1] = {0};
     zval arg_write[1] = {0};
     PGconn *pgsql;
     cb_item *pgGetItem = pg_get_item(fd);
     pgsql_link_handle *link;
     if (event & UV_WRITABLE && !pgGetItem->written && !PQisBusy(pgGetItem->conn))    //WRITE TO FD
     {
         printf("db zval handle is %p\n", &pgGetItem->db_handle);
         printf("written %d\n", pgGetItem->written);
         if (pgGetItem->written) {
//            g = pg_get_next_item(fd);
//            printf("written %d\n", g->written);
         }
         puts("Writable");
         zend_result pg_sent = FAILURE;
         ZVAL_COPY(&arg_write[0], &pgGetItem->db_handle);
         GC_TRY_ADDREF(Z_OBJ(pgGetItem->db_handle));
         puts("Copy success");
         call_php_fn(&pgGetItem->cb, 1, arg_write, &retval_write, "db_write_wait");

         if (Z_TYPE(retval_write) == IS_STRING) {
             link = pgsql_link_from_obj(Z_OBJ(pgGetItem->db_handle));

             char *query = Z_STRVAL(retval_write);
             if (link->conn == NULL) {
                 zend_throw_error(NULL, "PostgreSQL connection has already been closed");
             }
             pgsql = pgGetItem->conn;
             printf("%p | > %p \n", pgsql_link_from_obj(Z_OBJ(pgGetItem->db_handle))->conn, pgGetItem->conn);

             if (!PQsendQuery(pgsql, query)) {
                 puts(PQerrorMessage(pgsql));
                 puts("Retrying");
                 if (PQgetResult(pgsql) == NULL) {
                     if (!PQsendQuery(pgsql, query)) {
                         zend_throw_error(NULL, "Query can't be sent");
                         ZEND_ASSERT(EG(exception));
                         return false;
                     }
                 }
             }
             pg_sent = PQflush(pgsql);
             puts("Query sent");
             printf("sent status: %s\n", pg_sent == 0 ? "completed" : "error");
             pgGetItem->written = 1;
         } else {
             zend_type_error("Return value should be a string");
         }

     } else if (event & UV_READABLE && pgGetItem->written) {
         puts("Readable");
         if (pgGetItem->conn == NULL) {
             zend_throw_error(NULL, "PostgreSQL connection has already been closed");
             ZVAL_FALSE(&arg_read[0]);
         }

         pgsql = pgGetItem->conn;
         PGresult *pgsql_result = PQgetResult(pgsql);

         if (!pgsql_result) {
             zend_throw_error(NULL, "PostgreSQL no result closed");
             /* no result */
             ZVAL_FALSE(&arg_read[0]);
         }
         zend_class_entry * pgsql_result_ce = find_class_by_name("PgSql\\Result");
         if (!pgsql_result_ce) {
             zend_throw_error(NULL, "PostgreRes not found");
             /* no result */
             ZVAL_FALSE(&arg_read[0]);
         }
         object_init_ex(&arg_read[0], pgsql_result_ce);
         pgsql_result_handle *pg_result = pgsql_result_from_obj(Z_OBJ(arg_read[0]));
         pg_result->conn = (PGconn *) pgGetItem->conn;
         pg_result->result = pgsql_result;
         pg_result->row = 0;
//         puts("Here!6");
//         php_var_dump(&pgGetItem->cb_read.fci.function_name, 3);
//         php_var_dump(&pgGetItem->db_handle, 3);

         call_php_fn(&pgGetItem->cb_read, 1, arg_read, &retval, "db_wait");
//         puts("Here!7");
//         php_var_dump(&pgGetItem->cb_read.fci.function_name, 3);
//         php_var_dump(&pgGetItem->db_handle, 1);

         pg_get_and_remove_item(fd);
         puts("Here!8");
         PQclear(pgsql_result);
     }
}
static int prepare_poll_cb (int event, uint16_t fd){
    zval retval, retval_write;
    zval arg_read[1] = {0};
    zval arg_write[1] = {0};
    PGconn *pgsql;
    cb_item *pgGetItem = pg_get_item(fd);
    pgsql_link_handle *link;
    if (event & UV_WRITABLE && !pgGetItem->written && !PQisBusy(pgGetItem->conn))    //WRITE TO FD
    {
        printf("db zval handle is %p\n", &pgGetItem->db_handle);
        printf("written %d\n", pgGetItem->written);
        if (pgGetItem->written) {
//            g = pg_get_next_item(fd);
//            printf("written %d\n", g->written);
        }
        puts("Writable");
        zend_result pg_sent = FAILURE;
        ZVAL_COPY(&arg_write[0], &pgGetItem->db_handle);
        GC_TRY_ADDREF(Z_OBJ(pgGetItem->db_handle));
        puts("Copy success");
        call_php_fn(&pgGetItem->cb, 1, arg_write, &retval_write, "db_write_wait");

        if (Z_TYPE(retval_write) == IS_ARRAY) {
            link = pgsql_link_from_obj(Z_OBJ(pgGetItem->db_handle));


            if (link->conn == NULL) {
                zend_throw_error(NULL, "PostgreSQL connection has already been closed");
            }
            pgsql = pgGetItem->conn;
            printf("%p | > %p \n", pgsql_link_from_obj(Z_OBJ(pgGetItem->db_handle))->conn, pgGetItem->conn);
            HashTable *data = Z_ARRVAL(retval_write);
            zval * name = zend_hash_index_find_deref(data, 0);
            zval * query = zend_hash_index_find_deref(data, 1);
            if (Z_TYPE_P(name) == IS_STRING && Z_TYPE_P(query) == IS_STRING){
                if (!PQsendPrepare(pgsql, Z_STRVAL_P(name), Z_STRVAL_P(query), 0, NULL)) {
                    puts(PQerrorMessage(pgsql));
                    puts("Failed");
                    if (PQgetResult(pgsql) == NULL) {
                        if (!PQsendPrepare(pgsql, Z_STRVAL_P(name), Z_STRVAL_P(query), 0, NULL)) {
                            zend_throw_error(NULL, "Prepared query can't be sent");
                            ZEND_ASSERT(EG(exception));
                            return false;
                        }
                    }
                }
                pg_sent = PQflush(pgsql);
                puts("Query sent");
                printf("sent status: %s\n", pg_sent == 0 ? "completed" : "error");
                pgGetItem->written = 1;
            }

        } else {
            zend_type_error("Return value should be a  array of strings - [name, query]");
        }

    } else if (event & UV_READABLE && pgGetItem->written) {
        puts("Readable");
        if (pgGetItem->conn == NULL) {
            zend_throw_error(NULL, "PostgreSQL connection has already been closed");
            ZVAL_FALSE(&arg_read[0]);
        }

        pgsql = pgGetItem->conn;
        PGresult *pgsql_result = PQgetResult(pgsql);

        if (!pgsql_result) {
            zend_throw_error(NULL, "PostgreSQL no result closed");
            /* no result */
            ZVAL_FALSE(&arg_read[0]);
        }
        zend_class_entry * pgsql_result_ce = find_class_by_name("PgSql\\Result");
        if (!pgsql_result_ce) {
            zend_throw_error(NULL, "PostgreRes not found");
            /* no result */
            ZVAL_FALSE(&arg_read[0]);
        }
        object_init_ex(&arg_read[0], pgsql_result_ce);
        pgsql_result_handle *pg_result = pgsql_result_from_obj(Z_OBJ(arg_read[0]));
        pg_result->conn = (PGconn *) pgGetItem->conn;
        pg_result->result = pgsql_result;
        pg_result->row = 0;
//        puts("Here!6");
//        php_var_dump(&pgGetItem->cb_read.fci.function_name, 3);
//        php_var_dump(&pgGetItem->db_handle, 3);

        call_php_fn(&pgGetItem->cb_read, 1, arg_read, &retval, "db_wait");
//        puts("Here!7");
//        php_var_dump(&pgGetItem->cb_read.fci.function_name, 3);
//        php_var_dump(&pgGetItem->db_handle, 1);

        pg_get_and_remove_item(fd);
        puts("Here!8");
        PQclear(pgsql_result);
    }
}

void pg_poll_cb(uv_poll_t *handle, int status, int event) {
    struct fd *fd_container = handle->data;
    uint16_t fd = fd_container->fd;
    if (!pg_has_item(fd)) {
        uv_poll_stop(handle);
        efree(handle->data);
        efree(handle);
        return;
    }
  const char * f_name =  pg_get_func_name(fd);
//    f_name[0]='q';
//    for (int i = 0; i < 5; ++i) {
//        printf("b %d, ",f_name[i]);
//    }
//    puts(f_name);
//    printf("%p\n", f_name);
    if (strcmp(f_name, "query")==0){
       query_poll_cb(event, fd);
        return;
    } else if (strcmp(f_name, "prepare")==0){
        prepare_poll_cb(event, fd);
    }
}


ZEND_FUNCTION(pg_query_async) {
    zend_result pg_exists = FAILURE;
    zend_string * module_name = zend_string_init(PROP("pgsql"), 0);
    if (zend_hash_exists(&module_registry, module_name)) {
        zval * db;
        zend_fcall_info fci = empty_fcall_info;
        zend_fcall_info_cache fcc = empty_fcall_info_cache;
        zend_fcall_info fci_read = empty_fcall_info;
        zend_fcall_info_cache fcc_read = empty_fcall_info_cache;
        pgsql_link_handle *link;
        ZEND_PARSE_PARAMETERS_START(3, 3)
                Z_PARAM_ZVAL(db)
                Z_PARAM_FUNC(fci, fcc)
                Z_PARAM_FUNC(fci_read, fcc_read)ZEND_PARSE_PARAMETERS_END();

        link = pgsql_link_from_obj(Z_OBJ_P(db));
        if (link->conn == NULL) {
            zend_throw_error(NULL, "PostgreSQL connection has already been closed");
            RETURN_THROWS();
        }

        PGconn *pgsql = link->conn;
        int fd_number = PQsocket(pgsql);
        if (fd_number == -1) {
            zend_throw_error(NULL, "Can't allocate connection socket");
            RETURN_THROWS();
        }

#ifndef PG_PARALLEL
        puts("pll satrr1");
        PQsetnonblocking(pgsql, 1);
        uv_poll_t *handle = emalloc(sizeof(uv_poll_t));
        db_type_t *db_type = emalloc(sizeof(db_type_t));
        memset(db_type, 0, sizeof(db_type_t));
        printf("fd is %d\n", fd_number);
        init_cb(&fci, &fcc, &db_type->cb);
        init_cb(&fci_read, &fcc_read, &db_type->cb_read);
        db_type->type = PGSQL_DB;
        db_type->db_handle = db;
        uv_poll_init(MODULE_GL(loop), handle, fd_number);
        handle->data = db_type;
        puts("pll satrr");
        uv_poll_start(handle, UV_READABLE | UV_WRITABLE, poll_cb);
    //TODO rewrite to global struct with reqs number, fci, fcc's for writing and reading
#else
        cb_item item = {
                .read=0,
                .written=0,
                .type=PGSQL_DB,
                .conn=pgsql
        };
        ZVAL_COPY_VALUE(&item.db_handle, db);
        GC_TRY_ADDREF(Z_OBJ_P(db));
        init_cb(&fci, &fcc, &item.cb);
        init_cb(&fci_read, &fcc_read, &item.cb_read);
        printf("fd is %d\n", fd_number);
        pg_add_item("query", fd_number, item);

        if (!fd_map_has(fd_number)) {
            printf("fd not in map\n");
            fd_map_add(fd_number);
            PQsetnonblocking(pgsql, 1);
            uv_poll_t *handle = emalloc(sizeof(uv_poll_t));
            uv_poll_init(MODULE_GL(loop), handle, fd_number);
            struct fd fd_container = {.fd=fd_number, .name=QUERY};
            handle->data = emalloc(sizeof(struct fd));
            memcpy(handle->data, &fd_container, sizeof(struct fd));
            puts("poll started");
            uv_poll_start(handle, UV_READABLE | UV_WRITABLE, pg_poll_cb);
        } else {
            printf("fd is present\n");
        }
        printf("PgLink %p \n", link);
        RETURN_TRUE;
#endif
    } else {
        zend_throw_error(NULL, "PgSQL is not loaded\n");
        RETURN_THROWS();
    }
    zend_string_release(module_name);
}


ZEND_FUNCTION(pg_prepare_async) {
    zend_result pg_exists = FAILURE;
    zend_string * module_name = zend_string_init(PROP("pgsql"), 0);
    if (zend_hash_exists(&module_registry, module_name)) {
        zval * db;
        zend_fcall_info fci = empty_fcall_info;
        zend_fcall_info_cache fcc = empty_fcall_info_cache;
        zend_fcall_info fci_read = empty_fcall_info;
        zend_fcall_info_cache fcc_read = empty_fcall_info_cache;
        pgsql_link_handle *link;
        ZEND_PARSE_PARAMETERS_START(3, 3)
                Z_PARAM_ZVAL(db)
                Z_PARAM_FUNC(fci, fcc)
                Z_PARAM_FUNC(fci_read, fcc_read)ZEND_PARSE_PARAMETERS_END();
        PGconn *pgsql;
        link = pgsql_link_from_obj(Z_OBJ_P(db));
        if (link->conn == NULL) {
            zend_throw_error(NULL, "PostgreSQL connection has already been closed");
            RETURN_THROWS();
        }

        pgsql = link->conn;
        int fd_number = PQsocket(pgsql);
        if (fd_number == -1) {
            zend_throw_error(NULL, "Can't allocate connection socket");
            RETURN_THROWS();
        }

#ifndef PG_PARALLEL
        puts("pll satrr1");
        PQsetnonblocking(pgsql, 1);
        uv_poll_t *handle = emalloc(sizeof(uv_poll_t));
        db_type_t *db_type = emalloc(sizeof(db_type_t));
        memset(db_type, 0, sizeof(db_type_t));
        printf("fd is %d\n", fd_number);
        init_cb(&fci, &fcc, &db_type->cb);
        init_cb(&fci_read, &fcc_read, &db_type->cb_read);
        db_type->type = PGSQL_DB;
        db_type->db_handle = db;
        uv_poll_init(MODULE_GL(loop), handle, fd_number);
        handle->data = db_type;
        puts("pll satrr");
        uv_poll_start(handle, UV_READABLE | UV_WRITABLE, poll_cb);
    //TODO rewrite to global struct with reqs number, fci, fcc's for writing and reading
#else
        cb_item item = {
                .read=0,
                .written=0,
                .type=PGSQL_DB,
                .conn=pgsql
        };
        ZVAL_COPY_VALUE(&item.db_handle, db);
        GC_TRY_ADDREF(Z_OBJ_P(db));
        init_cb(&fci, &fcc, &item.cb);
        init_cb(&fci_read, &fcc_read, &item.cb_read);
        printf("g->cb_read %p\n", &item.cb_read.fci);
        printf("fd is %d\n", fd_number);
        pg_add_item("prepare", fd_number, item);

        if (!fd_map_has(fd_number)) {
            printf("fd not in map\n");
            fd_map_add(fd_number);
            PQsetnonblocking(pgsql, 1);
            uv_poll_t *handle = emalloc(sizeof(uv_poll_t));
            uv_poll_init(MODULE_GL(loop), handle, fd_number);
            struct fd fd_container = {.fd=fd_number, .name=PREPARE};
            handle->data = emalloc(sizeof(struct fd));
            memcpy(handle->data, &fd_container, sizeof(struct fd));
            puts("poll started");
            uv_poll_start(handle, UV_READABLE | UV_WRITABLE, pg_poll_cb);
        } else {
            printf("fd is present\n");
        }
        printf("PgLink %p \n", link);

#endif
    } else {
        zend_throw_error(NULL, "PgSQL is not loaded\n");
        RETURN_THROWS();
    }
    zend_string_release(module_name);
}


