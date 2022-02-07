//
// Created by admin on 13.12.2021.
//
#ifndef MYSQLI_USE_MYSQLND
#define MYSQLI_USE_MYSQLND
#endif
#include <php.h>
#include <uv.h>
#include <zend_API.h>
#include "db_async_override.h"
#include "mysqlnd/mysqlnd.h"
#include "../web/helpers.h"
#include "../../php_fileio.h"
#include "mysqlnd/mysqlnd_debug.h"
#include "../http/request.h"
#include "../common/call_php_fn.h"
#include "../common/struct.h"
#include "mysqli/php_mysqli_structs.h"
#include <libpq-fe.h>
#include <libpq/libpq-fs.h>
#include <zend_exceptions.h>

#define mysqlnd_async_query(conn, query_str, query_len)	((conn)->data)->m->send_query((conn)->data, (query_str), (query_len), MYSQLND_SEND_QUERY_EXPLICIT, NULL, NULL)
#define mysqlnd_reap_async_query(conn)					((conn)->data)->m->reap_query((conn)->data, MYSQLND_REAP_RESULT_EXPLICIT)

#define MYSQLI_STORE_RESULT 0
#define MYSQLI_USE_RESULT 	1
#define Z_MYSQLI(zv) php_mysqli_fetch_object(Z_OBJ((zv)))
typedef struct pgsql_link_handle {
	PGconn* conn;
	zend_string* hash;
	HashTable* notices;
	bool persistent;
	zend_object std;
} pgsql_link_handle;

typedef struct {
	MYSQLND* mysql;
	zend_string* hash_key;
	zval			li_read;
	php_stream* li_stream;
	unsigned int 	multi_query;
	bool		persistent;
	int				async_result_fetch_type;
} MY_MYSQLND;


typedef struct _php_pgsql_result_handle {
	PGconn* conn;
	PGresult* result;
	int row;
	zend_object std;
} pgsql_result_handle;

static inline pgsql_link_handle* pgsql_link_from_obj(zend_object* obj) {
	return (pgsql_link_handle*)((char*)(obj)-XtOffsetOf(pgsql_link_handle, std));
}

static inline pgsql_result_handle* pgsql_result_from_obj(zend_object* obj) {
	return (pgsql_result_handle*)((char*)(obj)-XtOffsetOf(pgsql_result_handle, std));
}

static zend_object* mysqli_objects_new(zend_class_entry* class_type)
{
	mysqli_object* intern;
	zend_class_entry* mysqli_base_class;
	zend_object_handlers* handlers;

	intern = zend_object_alloc(sizeof(mysqli_object), class_type);

	mysqli_base_class = class_type;
	while (mysqli_base_class->type != ZEND_INTERNAL_CLASS &&
		mysqli_base_class->parent != NULL) {
		mysqli_base_class = mysqli_base_class->parent;
	}
	intern->prop_handler = zend_hash_find_ptr(&classes, mysqli_base_class->name);

	zend_object_std_init(&intern->zo, class_type);
	object_properties_init(&intern->zo, class_type);

	/* link object */
	if (instanceof_function(class_type, mysqli_link_class_entry)) {
		handlers = &mysqli_object_link_handlers;
	}
	else if (instanceof_function(class_type, mysqli_driver_class_entry)) { /* driver object */
		handlers = &mysqli_object_driver_handlers;
	}
	else if (instanceof_function(class_type, mysqli_stmt_class_entry)) { /* stmt object */
		handlers = &mysqli_object_stmt_handlers;
	}
	else if (instanceof_function(class_type, mysqli_result_class_entry)) { /* result object */
		handlers = &mysqli_object_result_handlers;
	}
	else if (instanceof_function(class_type, mysqli_warning_class_entry)) { /* warning object */
		handlers = &mysqli_object_warning_handlers;
	}
	else {
		handlers = &mysqli_object_handlers;
	}

	intern->zo.handlers = handlers;

	return &intern->zo;
}

void poll_cb(uv_poll_t* handle, int event, int status) {
	puts("Polled");
	zval retval, retval_write;
	zval arg_read[1] = { 0 };
	zval arg_write[1] = { 0 };
	PGconn* pgsql;

	db_type_t* db_data = handle->data;
	zend_object* obj = Z_OBJ_P(db_data->db_handle);
	pgsql_link_handle* link;
	if (event & UV_WRITABLE && !db_data->written)	//WRITE TO FD
	{
		puts("Writable");
		zend_result pg_sent = FAILURE;


		switch (db_data->type) {
		case MYSQL_DB:
			ZVAL_COPY(&arg_write[0], db_data->db_handle);
			break;
		case PGSQL_DB:
			ZVAL_COPY(&arg_write[0], db_data->db_handle);
			break;
		}
		call_php_fn(&db_data->cb, 1, arg_read, &retval_write, "db_write_wait");
		if (Z_TYPE(retval_write) == IS_STRING) {
			switch (db_data->type) {
			case MYSQL_DB:
				//ZVAL_COPY(&arg_write[0], db_data->db_handle); //RUN send Query
				break;
			case PGSQL_DB:
				link = pgsql_link_from_obj(obj);
				//ZVAL_COPY(&arg_write[0], db_data->db_handle);
				char* query = Z_STRVAL(retval_write);

				if (link->conn == NULL) {
					zend_throw_error(NULL, "PostgreSQL connection has already been closed");

				}
				pgsql = link->conn;
				if (!PQsendQuery(pgsql, query)) {
					zend_throw_error(NULL, "Query can't be sent");
				}
				pg_sent = PQflush(pgsql);
				db_data->written = 1;
				break;
			}
		}
		else
		{
			zend_type_error("Return value should be a string");
		}

	}
	else if (event & UV_READABLE && db_data->written) {
		puts("Readable");
		uv_poll_stop(handle);
		switch (db_data->type) {
		case MYSQL_DB:
			ZVAL_COPY(&arg_read[0], db_data->db_handle);

			MY_MYSQL* mysql;
			zval* mysql_link = db_data->db_handle;
			MYSQLI_RESOURCE* result_resource;
			MYSQL_RES* result = NULL;

			MYSQLI_RESOURCE* my_res;
			mysqli_object* intern = Z_MYSQLI_P(mysql_link);
			if (!(my_res = (MYSQLI_RESOURCE*)intern->ptr)) {
				zend_throw_error(NULL, "%s object is already closed", ZSTR_VAL(intern->zo.ce->name));
				ZEND_ASSERT(EG(exception));
				break;
			}
			mysql = (MY_MYSQL*)my_res->ptr;
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
			ZVAL_OBJ(&arg_read[0], mysqli_objects_new(mysqli_result_class_entry));
			Z_MYSQLI(arg_read[0])->ptr = result_resource;
			break;
		case PGSQL_DB:
			link = pgsql_link_from_obj(obj);
			if (link->conn == NULL) {
				zend_throw_error(NULL, "PostgreSQL connection has already been closed");
				ZVAL_FALSE(&arg_read[0]);
			}
			pgsql = link->conn;
			PGresult* pgsql_result = PQgetResult(pgsql);
			pgsql_result_handle* pg_result;
			if (!pgsql_result) {
				zend_throw_error(NULL, "PostgreSQL no result closed");
				/* no result */
				ZVAL_FALSE(&arg_read[0]);
			}
			zend_string* lcname;
			zend_class_entry* pgsql_result_ce;
			char n[] = "PgSql\\Result";
			zend_string* name = zend_string_init(n, strlen(n), 0);
			if (1) {
				/* Ignore leading "\" */
				lcname = zend_string_tolower(name);
				pgsql_result_ce = zend_hash_find_ptr(EG(class_table), lcname);
				zend_string_release_ex(lcname, 0);
			}
			else {
				pgsql_result_ce = zend_lookup_class(name);
			}

			if (!pgsql_result_ce) {
				zend_throw_error(NULL, "PostgreRes not found");
				/* no result */
				ZVAL_FALSE(&arg_read[0]);
			}

			object_init_ex(&arg_read[0], pgsql_result_ce);
			pg_result = pgsql_result_from_obj(Z_OBJ(arg_read[0]));
			pg_result->conn = pgsql;
			pg_result->result = pgsql_result;
			pg_result->row = 0;
			break;
		}
		call_php_fn(&db_data->cb_read, 1, arg_read, &retval, "db_wait");
	}




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

}void poll_write_cb(uv_poll_t* handle, int event, int status) {
	puts("Writable");
	zval retval;
	zval arg[1] = { 0 };
	PGconn* pgsql;
	uv_poll_stop(handle);
	db_type_t* db_data = handle->data;
	zend_object* obj = Z_OBJ_P(db_data->db_handle);
	pgsql_link_handle* link;
	zend_result pg_sent = FAILURE;


	switch (db_data->type) {
	case MYSQL_DB:
		ZVAL_COPY(&arg[0], db_data->db_handle);
		break;
	case PGSQL_DB:
		//link = pgsql_link_from_obj(obj);
		ZVAL_COPY(&arg[0], db_data->db_handle);

		//if (link->conn == NULL) {
		//    zend_throw_error(NULL, "PostgreSQL connection has already been closed");
		//    ZVAL_FALSE(&arg[0]);
		//}
		//pgsql = link->conn;
		// if (!PQsendQuery(pgsql, query)) {
		//RETURN_FALSE;
		//}
		//pg_sent = PQflush(pgsql);
		break;
	}

	call_php_fn(&db_data->cb, 1, arg, &retval, "db_write_wait");

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
	MYSQLND* p = NULL;
	MY_MYSQL* obj;
	zval* db;
	zend_fcall_info fci = empty_fcall_info;
	zend_fcall_info_cache fcc = empty_fcall_info_cache;
	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_ZVAL(db)
		Z_PARAM_FUNC(fci, fcc)ZEND_PARSE_PARAMETERS_END();
	MYSQLI_FETCH_RESOURCE_CONN(obj, db, MYSQLI_STATUS_VALID);
	p = obj->mysql;
	php_stream* stream = p->data->vio->data->m.get_stream((p)->data->vio);
	//    DBG_INF_FMT("conn=%"
	//    PRIu64
	//    " stream=%p", p->data->thread_id, stream);
	zend_result res;
	set_non_blocking(stream);
	int fd = cast_to_fd(stream, &res);
	uv_poll_t* handle = emalloc(sizeof(uv_poll_t));
	db_type_t* db_type = emalloc(sizeof(db_type_t));
	printf("fd is %d\n", fd);
	init_cb(&fci, &fcc, &db_type->cb);
	db_type->type = MYSQL_DB;
	db_type->db_handle = db;
	uv_poll_init(MODULE_GL(loop), handle, fd);
	handle->data = db_type;
	uv_poll_start(handle, UV_READABLE, poll_cb);
}

ZEND_FUNCTION(pg_wait) {
	zend_result pg_exists = FAILURE;
	zend_string* module_name = zend_string_init(PROP("pgsql"), 0);
	if (zend_hash_exists(&module_registry, module_name)) {
		zval* db;
		zend_fcall_info fci = empty_fcall_info;
		zend_fcall_info_cache fcc = empty_fcall_info_cache;
		zend_fcall_info fci_read = empty_fcall_info;
		zend_fcall_info_cache fcc_read = empty_fcall_info_cache;
		pgsql_link_handle* link;
		ZEND_PARSE_PARAMETERS_START(2, 2)
			Z_PARAM_ZVAL(db)
			Z_PARAM_FUNC(fci, fcc)
			Z_PARAM_FUNC(fci_read, fcc_read)
			ZEND_PARSE_PARAMETERS_END();

		PGconn* pgsql;
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
		PQsetnonblocking(pgsql, 1);
		uv_poll_t* handle = emalloc(sizeof(uv_poll_t));
		db_type_t* db_type = emalloc(sizeof(db_type));
		memset(db_type, 0, sizeof(db_type));
		printf("fd is %d\n", fd_number);
		init_cb(&fci, &fcc, &db_type->cb);
		init_cb(&fci_read, &fcc_read, &db_type->cb_read);
		db_type->type = PGSQL_DB;
		db_type->db_handle = db;
		uv_poll_init(MODULE_GL(loop), handle, fd_number);
		handle->data = db_type;
		uv_poll_start(handle, UV_READABLE | UV_WRITABLE, poll_cb);
	}
	else {
		zend_throw_error(NULL, "PgSQL is not loaded\n");
		RETURN_THROWS();
	}
	zend_string_release(module_name);
}

ZEND_FUNCTION(pg_write_wait) {
	zend_result pg_exists = FAILURE;
	zend_string* module_name = zend_string_init(PROP("pgsql"), 0);
	if (zend_hash_exists(&module_registry, module_name)) {
		zval* db;
		zend_fcall_info fci = empty_fcall_info;
		zend_fcall_info_cache fcc = empty_fcall_info_cache;
		pgsql_link_handle* link;
		ZEND_PARSE_PARAMETERS_START(2, 2)
			Z_PARAM_ZVAL(db)
			Z_PARAM_FUNC(fci, fcc)ZEND_PARSE_PARAMETERS_END();
		PGconn* pgsql;
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
		PQsetnonblocking(pgsql, 1);
		uv_poll_t* handle = emalloc(sizeof(uv_poll_t));
		db_type_t* db_type = emalloc(sizeof(db_type));
		printf("fd is %d\n", fd_number);
		init_cb(&fci, &fcc, &db_type->cb);
		db_type->type = PGSQL_DB;
		db_type->db_handle = db;
		uv_poll_init(MODULE_GL(loop), handle, fd_number);
		handle->data = db_type;
		uv_poll_start(handle, UV_WRITABLE, poll_write_cb);
	}
	else {
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
	zend_class_entry* mysqli_link_class_entry = zend_hash_str_find_ptr(CG(class_table), PROP("mysqli"));
	zend_class_entry* mysqli_result_class_entrqy = zend_hash_str_find_ptr(CG(class_table), PROP("mysqli_result"));
	INIT_CLASS_ENTRY(ce, "mysqli_async", class_Promise_methods);
	MODULE_GL(mysqli_async_class) = zend_register_internal_class_ex(&ce, mysqli_link_class_entry);
}
