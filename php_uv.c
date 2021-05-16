/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2012 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Shuhei Tanuma <chobieeee@php.net>                           |
   |          Bob Weinand <bobwei9@hotmail.com>                           |
   +----------------------------------------------------------------------+
 */

#pragma GCC diagnostic ignored "-Wmissing-braces"

#include "php_uv.h"
#include "php_main.h"
#include "ext/standard/info.h"
#include "zend_smart_str.h"
#include "uv_args.h"
#ifndef PHP_UV_DEBUG
#define PHP_UV_DEBUG 0
#endif

#if defined(ZTS) && PHP_VERSION_ID < 80000
#undef TSRMLS_C
#undef TSRMLS_CC
#undef TSRMLS_D
#undef TSRMLS_DC
#define TSRMLS_C tsrm_ls
#define TSRMLS_CC , TSRMLS_C
#define TSRMLS_D void *tsrm_ls
#define TSRMLS_DC , TSRMLS_D

#ifdef COMPILE_DL_UV
ZEND_TSRMLS_CACHE_DEFINE()
#endif
#endif

ZEND_DECLARE_MODULE_GLOBALS(uv);

#ifndef GC_ADDREF
	#define GC_ADDREF(ref) ++GC_REFCOUNT(ref)
#endif

#if PHP_VERSION_ID < 70100
	#define uv_zend_wrong_parameter_class_error(throw, ...) zend_wrong_paramer_class_error(__VA_ARGS__)
#elif PHP_VERSION_ID < 70200 || PHP_VERSION_ID >= 70300
	#define uv_zend_wrong_parameter_class_error(throw, ...) zend_wrong_parameter_class_error(__VA_ARGS__)
#else
	#define uv_zend_wrong_parameter_class_error(...) zend_wrong_parameter_class_error(__VA_ARGS__)
#endif

#if PHP_VERSION_ID < 70200
	#define UV_PARAM_PROLOGUE Z_PARAM_PROLOGUE(0)
#else
	#define UV_PARAM_PROLOGUE Z_PARAM_PROLOGUE(0, 0)
#endif

#if PHP_VERSION_ID < 70400
	#define _error_code error_code
#endif

#if PHP_VERSION_ID >= 80000
#define zend_internal_type_error(strict_types, ...) zend_type_error(__VA_ARGS__)
#endif

#define UV_PARAM_OBJ_EX(dest, type, check_null, ce, ...) \
	{ \
		zval *zv; \
		UV_PARAM_PROLOGUE \
		if (UNEXPECTED(!uv_parse_arg_object(_arg, &zv, check_null, ce, ##__VA_ARGS__, NULL))) { \
			if (!(_flags & ZEND_PARSE_PARAMS_QUIET)) { \
				zend_string *names = php_uv_concat_ce_names(ce, ##__VA_ARGS__, NULL); \
				uv_zend_wrong_parameter_class_error(_flags & ZEND_PARSE_PARAMS_THROW, _i, ZSTR_VAL(names), _arg); \
				zend_string_release(names); \
			} \
			_error_code = ZPP_ERROR_FAILURE; \
			break; \
		} \
		if (GC_FLAGS(Z_OBJ_P(zv)) & IS_OBJ_DESTRUCTOR_CALLED) { \
			if (!(_flags & ZEND_PARSE_PARAMS_QUIET)) { \
				php_error_docref(NULL, E_WARNING, "passed %s handle is already closed", ZSTR_VAL(Z_OBJCE_P(_arg)->name)); \
			} \
			_error_code = ZPP_ERROR_FAILURE; \
			break; \
		} \
		dest = zv == NULL ? NULL : (type *) Z_OBJ_P(zv); \
	}

#define UV_PARAM_OBJ(dest, type, ...) UV_PARAM_OBJ_EX(dest, type, 0, ##__VA_ARGS__, NULL)
#define UV_PARAM_OBJ_NULL(dest, type, ...) UV_PARAM_OBJ_EX(dest, type, 1, ##__VA_ARGS__, NULL)

static ZEND_COLD zend_string *php_uv_concat_ce_names(zend_class_entry *ce, zend_class_entry *next, ...) {
	va_list va;
	smart_str buf = {0};

	va_start(va, next);

	if (!next) {
		return zend_string_copy(ce->name);
	}

	goto start;
	do {
		if (next) {
			smart_str_appends(&buf, ", ");
		} else {
			smart_str_appends(&buf, " or ");
		}
start:
		smart_str_append(&buf, ce->name);
		ce = next;
		next = (zend_class_entry *) va_arg(va, zend_class_entry *);
	} while (next);

	va_end(va);

	smart_str_0(&buf);
	return buf.s;
}

/* gcc complains: sorry, unimplemented: function ‘uv_parse_arg_object’ can never be inlined because it uses variable argument lists */
#ifdef __clang__
static zend_always_inline int uv_parse_arg_object(zval *arg, zval **dest, int check_null, zend_class_entry *ce, ...) {
#else
static int uv_parse_arg_object(zval *arg, zval **dest, int check_null, zend_class_entry *ce, ...) {
#endif
	if (EXPECTED(Z_TYPE_P(arg) == IS_OBJECT)) {
		va_list va;
		zend_class_entry *argce = Z_OBJCE_P(arg);
		va_start(va, ce);
		do {
			if (instanceof_function(argce, ce)) {
				*dest = arg;
				return 1;
			}
			ce = (zend_class_entry *) va_arg(va, zend_class_entry *);
		} while (ce);
	} else if (check_null && EXPECTED(Z_TYPE_P(arg) == IS_NULL)) {
		*dest = NULL;
		return 1;
	}
	return 0;
}

#define PHP_UV_DEINIT_UV(uv) \
	clean_uv_handle(uv); \
	OBJ_RELEASE(&uv->std);

#define PHP_UV_INIT_GENERIC(dest, type, ce) \
	do { \
		zval zv; \
		object_init_ex(&zv, ce); \
		dest = (type *) Z_OBJ(zv); \
	} while (0)

#define PHP_UV_INIT_UV(uv, ce) PHP_UV_INIT_GENERIC(uv, php_uv_t, ce)

#define PHP_UV_INIT_UV_EX(_uv, ce, cb, ...) \
	do { \
		int r; \
		PHP_UV_INIT_UV(_uv, ce); \
		r = cb(&loop->loop, (void *) &_uv->uv.handle, ##__VA_ARGS__); \
		if (r) { \
			PHP_UV_DEINIT_UV(_uv); \
			php_error_docref(NULL, E_WARNING, #cb " failed"); \
			RETURN_FALSE; \
		} \
	} while (0)

#define PHP_UV_INIT_CONNECT(req, uv) \
	req = (uv_connect_t *) emalloc(sizeof(uv_connect_t)); \
	req->data = uv;

#define PHP_UV_INIT_WRITE_REQ(w, uv, str, strlen, cb) \
	w = (write_req_t *) emalloc(sizeof(write_req_t)); \
	w->req.data = uv; \
	w->buf = uv_buf_init(estrndup(str, strlen), strlen); \
	w->cb = cb; \

#define PHP_UV_INIT_SEND_REQ(w, uv, str, strlen) \
	w = (send_req_t *) emalloc(sizeof(send_req_t)); \
	w->req.data = uv; \
	w->buf = uv_buf_init(estrndup(str, strlen), strlen); \

#define PHP_UV_FETCH_UV_DEFAULT_LOOP(loop) \
	if (loop == NULL) { \
		loop = php_uv_default_loop(); \
	}  \

#define PHP_UV_INIT_LOCK(lock, lock_type) \
	PHP_UV_INIT_GENERIC(lock, php_uv_lock_t, uv_lock_ce); \
	lock->type = lock_type;

#define PHP_UV_CHECK_VALID_FD(fd, zstream) \
	if (fd < 0) { \
		php_error_docref(NULL, E_WARNING, "invalid variable passed. can't convert to fd."); \
		PHP_UV_DEINIT_UV(uv); \
		RETURN_FALSE; \
	} \
	if (Z_ISUNDEF(uv->fs_fd)) { \
		ZVAL_COPY(&uv->fs_fd, zstream); \
	}

#define PHP_UV_ZVAL_TO_VALID_POLL_FD(fd, zstream) \
{ \
	fd = php_uv_zval_to_valid_poll_fd(zstream); \
	PHP_UV_CHECK_VALID_FD(fd, zstream) \
}

#define PHP_UV_ZVAL_TO_FD(fd, zstream) \
{ \
	fd = php_uv_zval_to_fd(zstream); \
	PHP_UV_CHECK_VALID_FD(fd, zstream) \
}

#define PHP_UV_FS_ASYNC(loop, func,  ...) \
	error = uv_fs_open(&loop->loop, (uv_fs_t*)&uv->uv.fs, path->val, flag, mode, php_uv_fs_cb); \
	if (error) { \
		PHP_UV_DEINIT_UV(uv); \
		php_error_docref(NULL, E_WARNING, "uv_" #func " failed"); \
		return; \
	}

#define PHP_UV_INIT_ZVALS(uv) \
	{ \
		int ix = 0;\
		for (ix = 0; ix < PHP_UV_CB_MAX; ix++) {\
			uv->callback[ix] = NULL;\
		} \
		ZVAL_UNDEF(&uv->fs_fd); \
		ZVAL_UNDEF(&uv->fs_fd_alt); \
	}

#if PHP_VERSION_ID < 70300
 #define PHP_UV_SKIP_DTOR(uv) do { GC_FLAGS(&uv->std) |= IS_OBJ_DESTRUCTOR_CALLED; } while (0)
#else
 #define PHP_UV_SKIP_DTOR(uv) do { GC_ADD_FLAGS(&uv->std, IS_OBJ_DESTRUCTOR_CALLED); } while (0)
#endif
#define PHP_UV_IS_DTORED(uv) (GC_FLAGS(&uv->std) & IS_OBJ_DESTRUCTOR_CALLED)

#define PHP_UV_SOCKADDR_IPV4_INIT(sockaddr) PHP_UV_INIT_GENERIC(sockaddr, php_uv_sockaddr_t, uv_sockaddr_ipv4_ce);
#define PHP_UV_SOCKADDR_IPV6_INIT(sockaddr) PHP_UV_INIT_GENERIC(sockaddr, php_uv_sockaddr_t, uv_sockaddr_ipv6_ce);

#define PHP_UV_SOCKADDR_IS_IPV4(sockaddr) (sockaddr->std.ce == uv_sockaddr_ipv4_ce)
#define PHP_UV_SOCKADDR_IS_IPV6(sockaddr) (sockaddr->std.ce == uv_sockaddr_ipv6_ce)

#define PHP_UV_SOCKADDR_IPV4(sockaddr) sockaddr->addr.ipv4
#define PHP_UV_SOCKADDR_IPV4_P(sockaddr) &sockaddr->addr.ipv4

#define PHP_UV_SOCKADDR_IPV6(sockaddr) sockaddr->addr.ipv6
#define PHP_UV_SOCKADDR_IPV6_P(sockaddr) &sockaddr->addr.ipv6

#define PHP_UV_LOCK_RWLOCK_P(_lock) &_lock->lock.rwlock
#define PHP_UV_LOCK_MUTEX_P(_lock) &_lock->lock.mutex
#define PHP_UV_LOCK_SEM_P(_lock) &_lock->lock.semaphore

#define PHP_UV_FD_TO_ZVAL(zv, fd) { php_stream *_stream = php_stream_fopen_from_fd(fd, "w+", NULL); zval *_z = (zv); php_stream_to_zval(_stream, _z); }

#if PHP_UV_DEBUG>=1
#define PHP_UV_DEBUG_PRINT(format, ...) fprintf(stderr, format, ## __VA_ARGS__)
#else
#define PHP_UV_DEBUG_PRINT(format, ...)
#endif

#if PHP_UV_DEBUG>=1
#define PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(handler, uv) \
	{ \
		PHP_UV_DEBUG_PRINT("# %s add(%p - %s): %u->%u\n", #handler, uv, ZSTR_VAL(uv->std.ce->name), GC_REFCOUNT(&(uv)->std) - 1, GC_REFCOUNT(&(uv)->std)); \
	}
#define PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(handler, uv) \
	{ \
		PHP_UV_DEBUG_PRINT("# %s del(%p - %s): %u->%u\n", #handler, uv, ZSTR_VAL(uv->std.ce->name), GC_REFCOUNT(&(uv)->std), GC_REFCOUNT(&(uv)->std) - 1); \
	}
#else
#define PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(hander, uv)
#define PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(hander, uv)
#endif

#if defined(ZTS) && PHP_VERSION_ID < 80000
#define UV_FETCH_ALL(ls, id, type) ((type) (*((void ***) ls))[TSRM_UNSHUFFLE_RSRC_ID(id)])
#define UV_FETCH_CTX(ls, id, type, element) (((type) (*((void ***) ls))[TSRM_UNSHUFFLE_RSRC_ID(id)])->element)
#define UV_CG(ls, v)  UV_FETCH_CTX(ls, compiler_globals_id, zend_compiler_globals*, v)
#define UV_CG_ALL(ls) UV_FETCH_ALL(ls, compiler_globals_id, zend_compiler_globals*)
#define UV_EG(ls, v)  UV_FETCH_CTX(ls, executor_globals_id, zend_executor_globals*, v)
#define UV_SG(ls, v)  UV_FETCH_CTX(ls, sapi_globals_id, sapi_globals_struct*, v)
#define UV_EG_ALL(ls) UV_FETCH_ALL(ls, executor_globals_id, zend_executor_globals*)
#endif

#if !defined(PHP_WIN32) && !(defined(HAVE_SOCKETS) && !defined(COMPILE_DL_SOCKETS))
# if PHP_VERSION_ID >= 80000
zend_class_entry *(*socket_ce)(void) = NULL;
# else
int (*php_sockets_le_socket)(void) = NULL;
# endif
#endif

/* objects */
extern void php_uv_init(zend_class_entry *uv_ce);

static zend_object_handlers uv_default_handlers;

static zend_class_entry *uv_ce;
static zend_object_handlers uv_handlers;

static zend_class_entry *uv_stream_ce;

static zend_class_entry *uv_tcp_ce;
static zend_class_entry *uv_udp_ce;
static zend_class_entry *uv_pipe_ce;
static zend_class_entry *uv_idle_ce;
static zend_class_entry *uv_timer_ce;
static zend_class_entry *uv_async_ce;
static zend_class_entry *uv_addrinfo_ce;
static zend_class_entry *uv_process_ce;
static zend_class_entry *uv_prepare_ce;
static zend_class_entry *uv_check_ce;
static zend_class_entry *uv_work_ce;
static zend_class_entry *uv_fs_ce;
static zend_class_entry *uv_fs_event_ce;
static zend_class_entry *uv_tty_ce;
static zend_class_entry *uv_fs_poll_ce;
static zend_class_entry *uv_poll_ce;
static zend_class_entry *uv_signal_ce;

static zend_class_entry *uv_loop_ce;
static zend_object_handlers uv_loop_handlers;

static zend_class_entry *uv_sockaddr_ce;

static zend_class_entry *uv_sockaddr_ipv4_ce;
static zend_class_entry *uv_sockaddr_ipv6_ce;

static zend_class_entry *uv_lock_ce;
static zend_object_handlers uv_lock_handlers;

static zend_class_entry *uv_stdio_ce;
static zend_object_handlers uv_stdio_handlers;


typedef struct {
	uv_write_t req;
	uv_buf_t buf;
	php_uv_cb_t *cb;
} write_req_t;

typedef struct {
	uv_udp_send_t req;
	uv_buf_t buf;
} send_req_t;

enum php_uv_socket_type {
	PHP_UV_TCP_IPV4 = 1,
	PHP_UV_TCP_IPV6 = 2,
	PHP_UV_TCP      = 3,
	PHP_UV_UDP_IPV4 = 16,
	PHP_UV_UDP_IPV6 = 32,
	PHP_UV_UDP      = 48,
};

/* declarations */

static void php_uv_fs_cb(uv_fs_t* req);
/**
 * execute callback
 *
 * @param zval* retval_ptr non-initialized pointer. this will be allocate from zend_call_function
 * @param php_uv_cb_t* callback callable object
 * @param zval* params parameters.
 * @param int param_count
 * @return int (maybe..)
 */
static int php_uv_do_callback(zval *retval_ptr, php_uv_cb_t *callback, zval *params, int param_count TSRMLS_DC);

void static destruct_uv(zend_object *obj);
void static clean_uv_handle(php_uv_t *uv);

static void php_uv_tcp_connect_cb(uv_connect_t *conn_req, int status);

static void php_uv_write_cb(uv_write_t* req, int status);

static void php_uv_listen_cb(uv_stream_t* server, int status);

static void php_uv_shutdown_cb(uv_shutdown_t* req, int status);

static void php_uv_read_cb(uv_stream_t* handle, ssize_t nread,const uv_buf_t* buf);

/* unused: static void php_uv_read2_cb(uv_pipe_t* handle, ssize_t nread, uv_buf_t buf, uv_handle_type pending); */

static void php_uv_read_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

static void php_uv_close(php_uv_t *uv);

static void php_uv_timer_cb(uv_timer_t *handle);

static void php_uv_idle_cb(uv_timer_t *handle);

static void php_uv_signal_cb(uv_signal_t *handle, int sig_num);


static php_uv_loop_t *php_uv_default_loop()
{
	if (UV_G(default_loop) == NULL) {
		zval zv;
		object_init_ex(&zv, uv_loop_ce);
		UV_G(default_loop) = (php_uv_loop_t *) Z_OBJ(zv);
	}

	return UV_G(default_loop);
}

static php_socket_t php_uv_zval_to_valid_poll_fd(zval *ptr)
{
	php_socket_t fd = -1;
	php_stream *stream;

	/* Validate Checks */

#if !defined(PHP_WIN32) || (defined(HAVE_SOCKETS) && !defined(COMPILE_DL_SOCKETS))
	php_socket *socket;
#endif
	/* TODO: is this correct on windows platform? */
	if (Z_TYPE_P(ptr) == IS_RESOURCE) {
		if ((stream = (php_stream *) zend_fetch_resource_ex(ptr, NULL, php_file_le_stream()))) {
			/* make sure only valid resource streams are passed - plainfiles and most php streams are invalid */
			if (stream->wrapper && !strcmp((char *)stream->wrapper->wops->label, "PHP") && (!stream->orig_path || (strncmp(stream->orig_path, "php://std", sizeof("php://std") - 1) && strncmp(stream->orig_path, "php://fd", sizeof("php://fd") - 1)))) {
				php_error_docref(NULL, E_WARNING, "invalid resource passed, this resource is not supported");
				return -1;
			}

			/* Some streams (specifically STDIO and encrypted streams) can be cast to FDs */
			if (php_stream_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL, (void*)&fd, 1) == SUCCESS && fd >= 0) {
				if (stream->wrapper && !strcmp((char *)stream->wrapper->wops->label, "plainfile")) {
#ifndef PHP_WIN32
					struct stat stat;
					fstat(fd, &stat);
					if (!S_ISFIFO(stat.st_mode))
#endif
					{
						php_error_docref(NULL, E_WARNING, "invalid resource passed, this plain files are not supported");
						return -1;
					}
				}
				return fd;
			}

			fd = -1;
#if PHP_VERSION_ID < 80000 && (!defined(PHP_WIN32) || (defined(HAVE_SOCKETS) && !defined(COMPILE_DL_SOCKETS)))
		} else if (php_sockets_le_socket && (socket = (php_socket *) zend_fetch_resource_ex(ptr, NULL, php_sockets_le_socket()))) {
			fd = socket->bsd_socket;
#endif
		} else {
			php_error_docref(NULL, E_WARNING, "unhandled resource type detected.");
			fd = -1;
		}
#if PHP_VERSION_ID >= 80000 && (!defined(PHP_WIN32) || (defined(HAVE_SOCKETS) && !defined(COMPILE_DL_SOCKETS)))
	} else if (socket_ce && Z_TYPE_P(ptr) == IS_OBJECT && Z_OBJCE_P(ptr) == socket_ce && (socket = (php_socket *) ((char *)(Z_OBJ_P(ptr)) - XtOffsetOf(php_socket, std)))) {
		fd = socket->bsd_socket;
#endif
	}

	return fd;
}

static php_socket_t php_uv_zval_to_fd(zval *ptr)
{
	php_socket_t fd = -1;
	php_stream *stream;
#if !defined(PHP_WIN32) || (defined(HAVE_SOCKETS) && !defined(COMPILE_DL_SOCKETS))
	php_socket *socket;
#endif
	/* TODO: is this correct on windows platform? */
	if (Z_TYPE_P(ptr) == IS_RESOURCE) {
		if ((stream = (php_stream *) zend_fetch_resource_ex(ptr, NULL, php_file_le_stream()))) {
			if (php_stream_cast(stream, PHP_STREAM_AS_FD | PHP_STREAM_CAST_INTERNAL, (void *) &fd, 1) != SUCCESS || fd < 0) {
				fd = -1;
			}
#if PHP_VERSION_ID < 80000 && (!defined(PHP_WIN32) || (defined(HAVE_SOCKETS) && !defined(COMPILE_DL_SOCKETS)))
		} else if (php_sockets_le_socket && (socket = (php_socket *) zend_fetch_resource_ex(ptr, NULL, php_sockets_le_socket()))) {
			fd = socket->bsd_socket;
#endif
		} else {
			php_error_docref(NULL, E_WARNING, "unhandled resource type detected.");
			fd = -1;
		}
	} else if (Z_TYPE_P(ptr) == IS_LONG) {
		fd = Z_LVAL_P(ptr);
		if (fd < 0) {
			fd = -1;
		}

		{
			/* make sure that a valid resource handle was passed - issue #36 */
			int err = uv_guess_handle((uv_file) fd);
			if (err == UV_UNKNOWN_HANDLE) {
				php_error_docref(NULL, E_WARNING, "invalid resource type detected");
				fd = -1;
			}
		}
#if PHP_VERSION_ID >= 80000 && (!defined(PHP_WIN32) || (defined(HAVE_SOCKETS) && !defined(COMPILE_DL_SOCKETS)))
	} else if (socket_ce && Z_TYPE_P(ptr) == IS_OBJECT && Z_OBJCE_P(ptr) == socket_ce && (socket = (php_socket *) ((char *)(Z_OBJ_P(ptr)) - XtOffsetOf(php_socket, std)))) {
		fd = socket->bsd_socket;
#endif
	}

	return fd;
}

static const char* php_uv_strerror(long error_code)
{
	/* Note: uv_strerror doesn't use assert. we don't need check value here */
	return uv_strerror(error_code);
}

static php_uv_cb_t* php_uv_cb_init_dynamic(php_uv_t *uv, zend_fcall_info *fci, zend_fcall_info_cache *fcc) {
	php_uv_cb_t *cb = emalloc(sizeof(php_uv_cb_t));

	memcpy(&cb->fci, fci, sizeof(zend_fcall_info));
	memcpy(&cb->fcc, fcc, sizeof(zend_fcall_info_cache));

	if (ZEND_FCI_INITIALIZED(*fci)) {
		Z_TRY_ADDREF(cb->fci.function_name);
		if (fci->object) {
			GC_ADDREF(cb->fci.object);
		}
	}

	return cb;
}

static void php_uv_cb_init(php_uv_cb_t **result, php_uv_t *uv, zend_fcall_info *fci, zend_fcall_info_cache *fcc, enum php_uv_callback_type type)
{
	php_uv_cb_t *cb;

	if (uv->callback[type] == NULL) {
		cb = emalloc(sizeof(php_uv_cb_t));
	} else {
		cb = uv->callback[type];

		if (Z_TYPE(cb->fci.function_name) != IS_UNDEF) {
			zval_dtor(&cb->fci.function_name);
		}
		if (fci->object) {
			OBJ_RELEASE(fci->object);
		}
	}

	memcpy(&cb->fci, fci, sizeof(zend_fcall_info));
	memcpy(&cb->fcc, fcc, sizeof(zend_fcall_info_cache));

	if (ZEND_FCI_INITIALIZED(*fci)) {
		Z_TRY_ADDREF(cb->fci.function_name);
		if (fci->object) {
			GC_ADDREF(cb->fci.object);
		}
	}

	uv->callback[type] = cb;
}

static void php_uv_lock_init(enum php_uv_lock_type lock_type, INTERNAL_FUNCTION_PARAMETERS)
{
	php_uv_lock_t *lock = NULL;
	int error = 0;

	switch (lock_type) {
		case IS_UV_RWLOCK:
		case IS_UV_RWLOCK_WR:
		case IS_UV_RWLOCK_RD:
		{
			PHP_UV_INIT_LOCK(lock, IS_UV_RWLOCK);
			error = uv_rwlock_init(PHP_UV_LOCK_RWLOCK_P(lock));
		}
		break;
		case IS_UV_MUTEX:
		{
			PHP_UV_INIT_LOCK(lock, IS_UV_MUTEX);
			error = uv_mutex_init(PHP_UV_LOCK_MUTEX_P(lock));
		}
		break;
		case IS_UV_SEMAPHORE:
		{
			zend_long val = 0;

			if (zend_parse_parameters(ZEND_NUM_ARGS(),
				"l", &val) == FAILURE) {
				return;
			}

			PHP_UV_INIT_LOCK(lock, IS_UV_SEMAPHORE);
			error = uv_sem_init(PHP_UV_LOCK_SEM_P(lock), (int) val);
		}
		break;
		default:
			php_error_docref(NULL, E_ERROR, "unexpected type");
		break;
	}

	if (error == 0) {
		RETURN_OBJ(&lock->std);
	} else {
		OBJ_RELEASE(&lock->std);
		RETURN_FALSE;
	}
}

static void php_uv_lock_lock(enum php_uv_lock_type lock_type, INTERNAL_FUNCTION_PARAMETERS)
{
	php_uv_lock_t *lock;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		UV_PARAM_OBJ(lock, php_uv_lock_t, uv_lock_ce)
	ZEND_PARSE_PARAMETERS_END();

	switch (lock_type) {
		case IS_UV_RWLOCK:
		case IS_UV_RWLOCK_RD:
		{
			if (lock->locked == 0x01) {
				zend_error(E_WARNING, "Cannot acquire a read lock while holding a write lock");
				RETURN_FALSE;
			}

			uv_rwlock_rdlock(PHP_UV_LOCK_RWLOCK_P(lock));
			if (!lock->locked++) {
				lock->locked = 0x02;
			}
		}
		break;
		case IS_UV_RWLOCK_WR:
		{
			if (lock->locked) {
				zend_error(E_WARNING, "Cannot acquire a write lock when already holding a lock");
				RETURN_FALSE;
			}

			uv_rwlock_wrlock(PHP_UV_LOCK_RWLOCK_P(lock));
			lock->locked = 0x01;
		}
		break;
		case IS_UV_MUTEX:
		{
			uv_mutex_lock(PHP_UV_LOCK_MUTEX_P(lock));
			lock->locked = 0x01;
		}
		break;
		case IS_UV_SEMAPHORE:
		{
			uv_sem_post(PHP_UV_LOCK_SEM_P(lock));
		}
		break;
		default:
			php_error_docref(NULL, E_ERROR, "unexpected type");
		break;
	}
}

static void php_uv_lock_unlock(enum php_uv_lock_type  lock_type, INTERNAL_FUNCTION_PARAMETERS)
{
	php_uv_lock_t *lock;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		UV_PARAM_OBJ(lock, php_uv_lock_t, uv_lock_ce)
	ZEND_PARSE_PARAMETERS_END();

	switch (lock_type) {
		case IS_UV_RWLOCK:
		case IS_UV_RWLOCK_RD:
		{
			if (lock->locked > 0x01) {
				uv_rwlock_rdunlock(PHP_UV_LOCK_RWLOCK_P(lock));
				if (--lock->locked == 0x01) {
					lock->locked = 0x00;
				}
			}
		}
		break;
		case IS_UV_RWLOCK_WR:
		{
			if (lock->locked == 0x01) {
				uv_rwlock_wrunlock(PHP_UV_LOCK_RWLOCK_P(lock));
				lock->locked = 0x00;
			}
		}
		break;
		case IS_UV_MUTEX:
		{
			if (lock->locked == 0x01) {
				uv_mutex_unlock(PHP_UV_LOCK_MUTEX_P(lock));
				lock->locked = 0x00;
			}
		}
		break;
		case IS_UV_SEMAPHORE:
		{
			uv_sem_wait(PHP_UV_LOCK_SEM_P(lock));
		}
		break;
		default:
			php_error_docref(NULL, E_ERROR, "unexpected type");
		break;
	}
}

static void php_uv_lock_trylock(enum php_uv_lock_type lock_type, INTERNAL_FUNCTION_PARAMETERS)
{
	php_uv_lock_t *lock;
	int error = 0;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		UV_PARAM_OBJ(lock, php_uv_lock_t, uv_lock_ce)
	ZEND_PARSE_PARAMETERS_END();

	switch(lock_type) {
		case IS_UV_RWLOCK:
		case IS_UV_RWLOCK_RD:
		{
			if (lock->locked == 0x01) {
				zend_error(E_WARNING, "Cannot acquire a read lock while holding a write lock");
				RETURN_FALSE;
			}

			error = uv_rwlock_tryrdlock(PHP_UV_LOCK_RWLOCK_P(lock));
			if (error == 0) {
				if (!lock->locked++) {
					lock->locked = 0x02;
				}
				RETURN_TRUE;
			} else {
				RETURN_FALSE;
			}
		}
		break;
		case IS_UV_RWLOCK_WR:
		{
			if (lock->locked) {
				zend_error(E_WARNING, "Cannot acquire a write lock when already holding a lock");
				RETURN_FALSE;
			}

			error = uv_rwlock_trywrlock(PHP_UV_LOCK_RWLOCK_P(lock));
			if (error == 0) {
				lock->locked = 0x01;
				RETURN_TRUE;
			} else {
				RETURN_FALSE;
			}
		}
		break;
		case IS_UV_MUTEX:
		{
			error = uv_mutex_trylock(PHP_UV_LOCK_MUTEX_P(lock));

			if (error == 0) {
				lock->locked = 0x01;
				RETURN_TRUE;
			} else {
				RETURN_FALSE;
			}
		}
		break;
		case IS_UV_SEMAPHORE:
		{
			error = uv_sem_trywait(PHP_UV_LOCK_SEM_P(lock));
			RETURN_LONG(error);
		}
		default:
			php_error_docref(NULL, E_ERROR, "unexpected type");
		break;
	}
}


static void php_uv_fs_common(uv_fs_type fs_type, INTERNAL_FUNCTION_PARAMETERS)
{
	int error = 0;
	php_uv_loop_t *loop;
	php_uv_t *uv;
	zend_fcall_info fci       = empty_fcall_info;
	zend_fcall_info_cache fcc = empty_fcall_info_cache;
	php_uv_cb_t *cb;

#define PHP_UV_FS_PARSE_PARAMETERS_EX(num, params, required_cb) \
	ZEND_PARSE_PARAMETERS_START(1 + num + required_cb, 2 + num) \
		UV_PARAM_OBJ(loop, php_uv_loop_t, uv_loop_ce) \
		params \
		if (!required_cb) { \
			Z_PARAM_OPTIONAL \
		} \
		Z_PARAM_FUNC_EX(fci, fcc, 1, 0) \
	ZEND_PARSE_PARAMETERS_END()

#define PHP_UV_FS_PARSE_PARAMETERS(num, params) PHP_UV_FS_PARSE_PARAMETERS_EX(num, params, 0)	

#define PHP_UV_FS_SETUP() \
	PHP_UV_INIT_UV(uv, uv_fs_ce); \
	PHP_UV_FETCH_UV_DEFAULT_LOOP(loop); \
	php_uv_cb_init(&cb, uv, &fci, &fcc, PHP_UV_FS_CB);

#define PHP_UV_FS_SETUP_AND_EXECUTE(command, ...) \
	PHP_UV_FS_SETUP(); \
	PHP_UV_FS_ASYNC(loop, command, __VA_ARGS__);

	switch (fs_type) {
		case UV_FS_SYMLINK:
		{
			zend_string *from, *to;
			zend_long flags;

			PHP_UV_FS_PARSE_PARAMETERS(3, Z_PARAM_STR(from) Z_PARAM_STR(to) Z_PARAM_LONG(flags));
			PHP_UV_FS_SETUP_AND_EXECUTE(symlink, from->val, to->val, flags);
			break;
		}
		case UV_FS_LINK:
		{
			zend_string *from, *to;

			PHP_UV_FS_PARSE_PARAMETERS(2, Z_PARAM_STR(from) Z_PARAM_STR(to));
			PHP_UV_FS_SETUP_AND_EXECUTE(link, from->val, to->val);
			break;
		}
		case UV_FS_RENAME:
		{
			zend_string *from, *to;

			PHP_UV_FS_PARSE_PARAMETERS(2, Z_PARAM_STR(from) Z_PARAM_STR(to));
			PHP_UV_FS_SETUP_AND_EXECUTE(rename, from->val, to->val);
			break;
		}
		case UV_FS_UNLINK:
		{
			zend_string *path;

			PHP_UV_FS_PARSE_PARAMETERS(1, Z_PARAM_STR(path));
			PHP_UV_FS_SETUP_AND_EXECUTE(unlink, path->val);
			break;
		}
		case UV_FS_RMDIR:
		{
			zend_string *path;

			PHP_UV_FS_PARSE_PARAMETERS(1, Z_PARAM_STR(path));
			PHP_UV_FS_SETUP_AND_EXECUTE(rmdir, path->val);
			break;
		}
		case UV_FS_MKDIR:
		{
			zend_string *path;
			zend_long mode;

			PHP_UV_FS_PARSE_PARAMETERS(2, Z_PARAM_STR(path) Z_PARAM_LONG(mode));
			PHP_UV_FS_SETUP_AND_EXECUTE(mkdir, path->val, mode);
			break;
		}
		case UV_FS_FTRUNCATE:
		{
			zval *zstream = NULL;
			zend_long offset = 0;
			unsigned long fd;

			PHP_UV_FS_PARSE_PARAMETERS(2, Z_PARAM_RESOURCE(zstream) Z_PARAM_LONG(offset));
			PHP_UV_FS_SETUP()
			PHP_UV_ZVAL_TO_FD(fd, zstream);
			uv->fs_fd = *zstream;
			Z_ADDREF(uv->fs_fd);
			PHP_UV_FS_ASYNC(loop, ftruncate, fd, offset);
			break;
		}
		case UV_FS_FDATASYNC:
		{
			zval *zstream = NULL;
			unsigned long fd;

			PHP_UV_FS_PARSE_PARAMETERS(1, Z_PARAM_RESOURCE(zstream));
			PHP_UV_FS_SETUP()
			PHP_UV_ZVAL_TO_FD(fd, zstream);
			uv->fs_fd = *zstream;
			Z_ADDREF(uv->fs_fd);
			PHP_UV_FS_ASYNC(loop, fdatasync, fd);
			break;
		}
		case UV_FS_CLOSE:
		{
			zval *zstream = NULL;
			unsigned long fd;

			PHP_UV_FS_PARSE_PARAMETERS(1, Z_PARAM_RESOURCE(zstream));
			PHP_UV_FS_SETUP()
			PHP_UV_ZVAL_TO_FD(fd, zstream);
			uv->fs_fd = *zstream;
			Z_ADDREF(uv->fs_fd);
			PHP_UV_FS_ASYNC(loop, close, fd);
			break;
		}
		case UV_FS_OPEN:
		{
			zend_string *path;
			zend_long flag, mode;

			PHP_UV_FS_PARSE_PARAMETERS_EX(3, Z_PARAM_STR(path) Z_PARAM_LONG(flag) Z_PARAM_LONG(mode), 1);
			PHP_UV_FS_SETUP_AND_EXECUTE(open, path->val, flag, mode);
			break;
		}
		case UV_FS_SCANDIR:
		{
			zend_string *path;
			zend_long flags = 0;

			ZEND_PARSE_PARAMETERS_START(3, 4)
				UV_PARAM_OBJ(loop, php_uv_loop_t, uv_loop_ce)
				Z_PARAM_STR(path)
				Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
				Z_PARAM_OPTIONAL
				Z_PARAM_LONG(flags)
			ZEND_PARSE_PARAMETERS_END();
			PHP_UV_FS_SETUP_AND_EXECUTE(scandir, path->val, flags);
			break;
		}
		case UV_FS_LSTAT:
		{
			zend_string *path;

			PHP_UV_FS_PARSE_PARAMETERS_EX(1, Z_PARAM_STR(path), 1);
			PHP_UV_FS_SETUP_AND_EXECUTE(lstat, path->val);
			break;
		}
		case UV_FS_FSTAT:
		{
			zval *zstream = NULL;
			unsigned long fd;

			PHP_UV_FS_PARSE_PARAMETERS_EX(1, Z_PARAM_RESOURCE(zstream), 1);
			PHP_UV_FS_SETUP()
			PHP_UV_ZVAL_TO_FD(fd, zstream);
			uv->fs_fd = *zstream;
			Z_ADDREF(uv->fs_fd);
			PHP_UV_FS_ASYNC(loop, fstat, fd);
			break;
		}
		case UV_FS_STAT:
		{
			zend_string *path;

			PHP_UV_FS_PARSE_PARAMETERS_EX(1, Z_PARAM_STR(path), 1);
			PHP_UV_FS_SETUP_AND_EXECUTE(stat, path->val);
			break;
		}
		case UV_FS_UTIME:
		{
			zend_long utime, atime;
			zend_string *path;

			PHP_UV_FS_PARSE_PARAMETERS(3, Z_PARAM_STR(path) Z_PARAM_LONG(utime) Z_PARAM_LONG(atime));
			PHP_UV_FS_SETUP_AND_EXECUTE(utime, path->val, utime, atime);
			break;
		}
		case UV_FS_FUTIME:
		{
			zval *zstream = NULL;
			zend_long utime, atime;
			unsigned long fd;

			PHP_UV_FS_PARSE_PARAMETERS(3, Z_PARAM_RESOURCE(zstream) Z_PARAM_LONG(utime) Z_PARAM_LONG(atime));
			PHP_UV_FS_SETUP()
			PHP_UV_ZVAL_TO_FD(fd, zstream);
			uv->fs_fd = *zstream;
			Z_ADDREF(uv->fs_fd);
			PHP_UV_FS_ASYNC(loop, futime, fd, utime, atime);
			break;
		}
		case UV_FS_READLINK:
		{
			zend_string *path;

			PHP_UV_FS_PARSE_PARAMETERS_EX(1, Z_PARAM_STR(path), 1);
			PHP_UV_FS_SETUP_AND_EXECUTE(readlink, path->val);
			break;
		}
		case UV_FS_READ:
		{
			zval *zstream = NULL;
			unsigned long fd;
			zend_long length;
			zend_long offset;
			uv_buf_t buf;

			PHP_UV_FS_PARSE_PARAMETERS_EX(3, Z_PARAM_RESOURCE(zstream) Z_PARAM_LONG(offset) Z_PARAM_LONG(length), 1);
			if (length <= 0) {
				length = 0;
			}
			if (offset < 0) {
				offset = 0;
			}
			PHP_UV_FS_SETUP()
			PHP_UV_ZVAL_TO_FD(fd, zstream);
			uv->fs_fd = *zstream;
			Z_ADDREF(uv->fs_fd);

			uv->buffer = (char*) emalloc(length);
			buf = uv_buf_init(uv->buffer, length);

			PHP_UV_FS_ASYNC(loop, read, fd, &buf, 1, offset);
			break;
		}
		case UV_FS_SENDFILE:
		{
			zval *z_instream, *z_outstream = NULL;
			unsigned long in_fd, out_fd;
			zend_long offset, length = 0;

			PHP_UV_FS_PARSE_PARAMETERS(4, Z_PARAM_RESOURCE(z_instream) Z_PARAM_RESOURCE(z_outstream) Z_PARAM_LONG(offset) Z_PARAM_LONG(length));
			PHP_UV_FS_SETUP()
			/* TODO */
			PHP_UV_ZVAL_TO_FD(in_fd, z_instream);
			PHP_UV_ZVAL_TO_FD(out_fd, z_outstream);
			uv->fs_fd = *z_outstream;
			Z_ADDREF(uv->fs_fd);
			uv->fs_fd_alt = *z_instream;
			Z_ADDREF(uv->fs_fd_alt);
			PHP_UV_FS_ASYNC(loop, sendfile, in_fd, out_fd, offset, length);
			break;
		}
		case UV_FS_WRITE:
		{
			zval *zstream = NULL;
			zend_string *buffer;
			zend_long fd, offset = -1;
			uv_buf_t uv_fs_write_buf_t;

			ZEND_PARSE_PARAMETERS_START(3, 5)
				UV_PARAM_OBJ(loop, php_uv_loop_t, uv_loop_ce)
				Z_PARAM_RESOURCE(zstream)
				Z_PARAM_STR(buffer)
				Z_PARAM_OPTIONAL
				Z_PARAM_LONG(offset)
				Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
			ZEND_PARSE_PARAMETERS_END();
			PHP_UV_FS_SETUP();
			PHP_UV_ZVAL_TO_FD(fd, zstream);
			uv->fs_fd = *zstream;
			Z_ADDREF(uv->fs_fd);
			uv->buffer = estrndup(buffer->val, buffer->len);

			/* TODO: is this right?! */
			uv_fs_write_buf_t = uv_buf_init(uv->buffer, buffer->len);
			PHP_UV_FS_ASYNC(loop, write, fd, &uv_fs_write_buf_t, 1, offset);
			break;
		}
		case UV_FS_UNKNOWN:
		case UV_FS_CUSTOM:
		default: {
			php_error_docref(NULL, E_ERROR, "type; %d does not support yet.", fs_type);
			break;
		}
	}

#undef PHP_UV_FS_PARSE_PARAMETERS
#undef PHP_UV_FS_SETUP
#undef PHP_UV_FS_SETUP_AND_EXECUTE

}
/* util */

static zval php_uv_address_to_zval(const struct sockaddr *addr)
{
	zval tmp = {0};
	char ip[INET6_ADDRSTRLEN];
	const struct sockaddr_in *a4;
	const struct sockaddr_in6 *a6;
	int port;

	array_init(&tmp);

	switch (addr->sa_family) {
		case AF_INET6:
		{
			a6 = (const struct sockaddr_in6 *)addr;
			uv_inet_ntop(AF_INET, &a6->sin6_addr, ip, sizeof ip);
			port = ntohs(a6->sin6_port);

			add_assoc_string_ex(&tmp, ZEND_STRL("address"), ip);
			add_assoc_long_ex(&tmp, ZEND_STRL("port"), port);
			add_assoc_string_ex(&tmp, ZEND_STRL("family"), "IPv6");
			break;
		}
		case AF_INET:
		{
			a4 = (const struct sockaddr_in *)addr;
			uv_inet_ntop(AF_INET, &a4->sin_addr, ip, sizeof ip);
			port = ntohs(a4->sin_port);

			add_assoc_string_ex(&tmp, ZEND_STRL("address"), ip);
			add_assoc_long_ex(&tmp, ZEND_STRL("port"), port);
			add_assoc_string_ex(&tmp, ZEND_STRL("family"), "IPv4");
			break;
		}
		default:
		break;
	}

	return tmp;
}

static zval php_uv_make_stat(const uv_stat_t *s)
{
	zval tmp = {0};
	array_init(&tmp);

	add_assoc_long_ex(&tmp, ZEND_STRL("dev"), s->st_dev);
	add_assoc_long_ex(&tmp, ZEND_STRL("ino"), s->st_ino);
	add_assoc_long_ex(&tmp, ZEND_STRL("mode"), s->st_mode);
	add_assoc_long_ex(&tmp, ZEND_STRL("nlink"), s->st_nlink);
	add_assoc_long_ex(&tmp, ZEND_STRL("uid"), s->st_uid);
	add_assoc_long_ex(&tmp, ZEND_STRL("gid"), s->st_gid);
	add_assoc_long_ex(&tmp, ZEND_STRL("rdev"), s->st_rdev);
	add_assoc_long_ex(&tmp, ZEND_STRL("size"), s->st_size);

#ifndef PHP_WIN32
	add_assoc_long_ex(&tmp, ZEND_STRL("blksize"), s->st_blksize);
	add_assoc_long_ex(&tmp, ZEND_STRL("blocks"), s->st_blocks);
#endif

	add_assoc_long_ex(&tmp, ZEND_STRL("atime"), s->st_atim.tv_sec);
	add_assoc_long_ex(&tmp, ZEND_STRL("mtime"), s->st_mtim.tv_sec);
	add_assoc_long_ex(&tmp, ZEND_STRL("ctime"), s->st_ctim.tv_sec);

	return tmp;
}

static inline zend_bool php_uv_closeable_type(php_uv_t *uv) {
	zend_class_entry *ce = uv->std.ce;
	return ce == uv_pipe_ce || ce == uv_tty_ce || ce == uv_tcp_ce || ce == uv_udp_ce || ce == uv_prepare_ce || ce == uv_check_ce || ce == uv_idle_ce || ce == uv_async_ce || ce == uv_timer_ce || ce == uv_process_ce || ce == uv_fs_event_ce || ce == uv_poll_ce || ce == uv_fs_poll_ce || ce == uv_signal_ce;
}

/* destructor */

void static destruct_uv_lock(zend_object *obj)
{
	php_uv_lock_t *lock = (php_uv_lock_t *) obj;

	if (lock->type == IS_UV_RWLOCK) {
		if (lock->locked == 0x01) {
			php_error_docref(NULL, E_NOTICE, "uv_rwlock: still locked resource detected; forcing wrunlock");
			uv_rwlock_wrunlock(PHP_UV_LOCK_RWLOCK_P(lock));
		} else if (lock->locked) {
			php_error_docref(NULL, E_NOTICE, "uv_rwlock: still locked resource detected; forcing rdunlock");
			while (--lock->locked > 0) {
				uv_rwlock_rdunlock(PHP_UV_LOCK_RWLOCK_P(lock));
			}
		}
		uv_rwlock_destroy(PHP_UV_LOCK_RWLOCK_P(lock));
	} else if (lock->type == IS_UV_MUTEX) {
		if (lock->locked == 0x01) {
			php_error_docref(NULL, E_NOTICE, "uv_mutex: still locked resource detected; forcing unlock");
			uv_mutex_unlock(PHP_UV_LOCK_MUTEX_P(lock));
		}
		uv_mutex_destroy(PHP_UV_LOCK_MUTEX_P(lock));
	} else if (lock->type == IS_UV_SEMAPHORE) {
		if (lock->locked == 0x01) {
			php_error_docref(NULL, E_NOTICE, "uv_sem: still locked resource detected; forcing unlock");
			uv_sem_post(PHP_UV_LOCK_SEM_P(lock));
		}
		uv_sem_destroy(PHP_UV_LOCK_SEM_P(lock));
	}
}

static void destruct_uv_loop_walk_cb(uv_handle_t* handle, void* arg) 
{
	php_uv_t *uv = (php_uv_t *) handle->data;
	if (!PHP_UV_IS_DTORED(uv)) { // otherwise we're already closing
		php_uv_close(uv);
	}
}

void static destruct_uv_loop(zend_object *obj)
{
	php_uv_loop_t *loop_obj = (php_uv_loop_t *) obj;
	uv_loop_t *loop = &loop_obj->loop;
	if (loop_obj != UV_G(default_loop)) {
		uv_stop(loop); /* in case we haven't stopped the loop yet otherwise ... */
		uv_run(loop, UV_RUN_DEFAULT); /* invalidate the stop ;-) */

		/* for proper destruction: close all handles, let libuv call close callback and then close and free the loop */
		uv_walk(loop, destruct_uv_loop_walk_cb, NULL);
		uv_run(loop, UV_RUN_DEFAULT);
		uv_loop_close(loop);
	}
	if (loop_obj->gc_buffer) {
		efree(loop_obj->gc_buffer);
	}
}

void static clean_uv_handle(php_uv_t *uv) {
	int i;

	/* for now */
	for (i = 0; i < PHP_UV_CB_MAX; i++) {
		php_uv_cb_t *cb = uv->callback[i];
		if (cb != NULL) {
			if (ZEND_FCI_INITIALIZED(cb->fci)) {
				zval_dtor(&cb->fci.function_name);

				if (cb->fci.object != NULL) {
					OBJ_RELEASE(cb->fci.object);
				}
			}

			efree(cb);
			cb = NULL;
		}
	}

	PHP_UV_SKIP_DTOR(uv);

	if (!Z_ISUNDEF(uv->fs_fd)) {
		zval_ptr_dtor(&uv->fs_fd);
		ZVAL_UNDEF(&uv->fs_fd);
		if (!Z_ISUNDEF(uv->fs_fd_alt)) {
			zval_ptr_dtor(&uv->fs_fd_alt);
			ZVAL_UNDEF(&uv->fs_fd_alt);
		}
	}
}

void static destruct_uv(zend_object *obj)
{
	php_uv_t *uv = (php_uv_t *) obj;

	PHP_UV_DEBUG_PRINT("# will be free: (obj: %p)\n", obj);

	if (!php_uv_closeable_type(uv)) {
		if (uv_cancel(&uv->uv.req) == UV_EBUSY) {
			GC_ADDREF(obj);
		}
		clean_uv_handle(uv);
	} else {
		php_uv_close(uv);
		/* cleaning happens in close_cb */
	}
}

void static php_uv_free_write_req(write_req_t *wr) {
	if (wr->cb) {
		if (ZEND_FCI_INITIALIZED(wr->cb->fci)) {
			zval_ptr_dtor(&wr->cb->fci.function_name);
			if (wr->cb->fci.object != NULL) {
				OBJ_RELEASE(wr->cb->fci.object);
			}
		}

		efree(wr->cb);
	}
	if (wr->buf.base) {
		efree(wr->buf.base);
	}
	efree(wr);
}

/* callback */
static int php_uv_do_callback(zval *retval_ptr, php_uv_cb_t *callback, zval *params, int param_count TSRMLS_DC)
{
	int error;

#if defined(ZTS) && PHP_VERSION_ID < 80000
	void *old = tsrm_set_interpreter_context(tsrm_ls);
#endif

	if (ZEND_FCI_INITIALIZED(callback->fci)) {
		callback->fci.params = params;
		callback->fci.retval = retval_ptr;
		callback->fci.param_count = param_count;
#if PHP_VERSION_ID < 80000
		callback->fci.no_separation = 1;
#endif

		error = zend_call_function(&callback->fci, &callback->fcc);
	} else {
		error = -1;
	}

#if defined(ZTS) && PHP_VERSION_ID < 80000
	tsrm_set_interpreter_context(old);
#endif

	return error;
}

static int php_uv_do_callback2(zval *retval_ptr, php_uv_t *uv, zval *params, int param_count, enum php_uv_callback_type type TSRMLS_DC)
{
	int error = 0;

#if defined(ZTS) && PHP_VERSION_ID < 80000
	void *old = tsrm_set_interpreter_context(tsrm_ls);
#endif
	if (ZEND_FCI_INITIALIZED(uv->callback[type]->fci)) {
		uv->callback[type]->fci.params        = params;
		uv->callback[type]->fci.retval        = retval_ptr;
		uv->callback[type]->fci.param_count   = param_count;
#if PHP_VERSION_ID < 80000
		uv->callback[type]->fci.no_separation = 1;
#endif

		if (zend_call_function(&uv->callback[type]->fci, &uv->callback[type]->fcc) != SUCCESS) {
			error = -1;
		}
	} else {
		error = -2;
	}

#if defined(ZTS) && PHP_VERSION_ID < 80000
	tsrm_set_interpreter_context(old);
#endif
	//zend_fcall_info_args_clear(&uv->callback[type]->fci, 0);

	if (EG(exception)) {
		switch (type) {
			case PHP_UV_FS_CB:
				uv_stop(uv->uv.fs.loop);
				break;
			case PHP_UV_GETADDR_CB:
				uv_stop(uv->uv.addrinfo.loop);
				break;
			case PHP_UV_AFTER_WORK_CB:
				uv_stop(uv->uv.work.loop);
				break;
			case PHP_UV_SHUTDOWN_CB:
				uv_stop(uv->uv.shutdown.handle->loop);
				break;
			case PHP_UV_SEND_CB:
				uv_stop(uv->uv.udp_send.handle->loop);
				break;
			case PHP_UV_CONNECT_CB:
			case PHP_UV_PIPE_CONNECT_CB:
				uv_stop(uv->uv.connect.handle->loop);
				break;
			default:
				uv_stop(uv->uv.handle.loop);
		}
	}

	return error;
}

#if defined(ZTS) && PHP_VERSION_ID < 80000

static int php_uv_do_callback3(zval *retval_ptr, php_uv_t *uv, zval *params, int param_count, enum php_uv_callback_type type)
{
	int error = 0;
	void *tsrm_ls, *old;
	zend_op_array *ops;
	zend_function fn, *old_fn;

	if (ZEND_FCI_INITIALIZED(uv->callback[type]->fci)) {
		tsrm_ls = tsrm_new_interpreter_context();
		old = tsrm_set_interpreter_context(tsrm_ls);

		PG(expose_php) = 0;
		PG(auto_globals_jit) = 0;

		php_request_startup();
		EG(current_execute_data) = NULL;
		EG(current_module) = phpext_uv_ptr;

		uv->callback[type]->fci.params        = params;
		uv->callback[type]->fci.retval        = retval_ptr;
		uv->callback[type]->fci.param_count   = param_count;
#if PHP_VERSION_ID < 80000
		uv->callback[type]->fci.no_separation = 1;
#endif
		uv->callback[type]->fci.object = NULL;
#if PHP_VERSION_ID >= 70300
		uv->callback[type]->fci.size = sizeof(zend_fcall_info);
#else
		uv->callback[type]->fcc.initialized = 1;
#endif

		uv->callback[type]->fcc.calling_scope = NULL;
		uv->callback[type]->fcc.called_scope = NULL;
		uv->callback[type]->fcc.object = NULL;

		if (!ZEND_USER_CODE(uv->callback[type]->fcc.function_handler->type)) {
			return error = -2;
		}

		fn = *(old_fn = uv->callback[type]->fcc.function_handler);
		uv->callback[type]->fcc.function_handler = &fn;

		ops = &fn.op_array;
#if PHP_VERSION_ID < 70400
		ops->run_time_cache = NULL;
#else
		ZEND_MAP_PTR_SET(ops->run_time_cache, NULL);
#endif
		if (ops->fn_flags) {
			ops->fn_flags &= ~ZEND_ACC_CLOSURE;
			ops->prototype = NULL;
		}

		zend_try {
			if (zend_call_function(&uv->callback[type]->fci, &uv->callback[type]->fcc) != SUCCESS) {
				error = -1;
			}
		} zend_catch {
			error = -1;
		} zend_end_try();

		// after PHP 7.4 this is arena allocated and automatically freed
#if PHP_VERSION_ID < 70400
		if (ops->run_time_cache && !ops->function_name) {
			efree(ops->run_time_cache);
		}
#endif

		uv->callback[type]->fcc.function_handler = old_fn;

		php_request_shutdown(NULL);
		tsrm_set_interpreter_context(old);
		tsrm_free_interpreter_context(tsrm_ls);
	} else {
		error = -2;
	}

	//zend_fcall_info_args_clear(&uv->callback[type]->fci, 0);

	return error;
}
#endif

static void php_uv_tcp_connect_cb(uv_connect_t *req, int status)
{
	zval retval = {0};
	zval params[2] = {0};
	php_uv_t *uv = (php_uv_t *) req->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	ZVAL_OBJ(&params[0], &uv->std);
	ZVAL_LONG(&params[1], status);

	php_uv_do_callback2(&retval, uv, params, 2, PHP_UV_CONNECT_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_udp_tcp_cb, uv);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	zval_ptr_dtor(&retval);
	efree(req);
}

static void php_uv_process_close_cb(uv_process_t* process, int64_t exit_status, int term_signal)
{
	zval retval = {0};
	zval params[3] = {0};
	php_uv_t *uv = (php_uv_t *) process->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	ZVAL_OBJ(&params[0], &uv->std);
	ZVAL_LONG(&params[1], exit_status);
	ZVAL_LONG(&params[2], term_signal);

	php_uv_do_callback2(&retval, uv, params, 3, PHP_UV_PROC_CLOSE_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_process_close_cb, uv);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);

	zval_ptr_dtor(&retval);
}


static void php_uv_pipe_connect_cb(uv_connect_t *req, int status)
{
	zval retval = {0};
	zval params[2] = {0};
	php_uv_t *uv = (php_uv_t*)req->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	ZVAL_OBJ(&params[0], &uv->std);
	ZVAL_LONG(&params[1], status);

	php_uv_do_callback2(&retval, uv, params, 2, PHP_UV_PIPE_CONNECT_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_pipe_connect_cb, uv);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	zval_ptr_dtor(&retval);
	efree(req);
}


static void php_uv_walk_cb(uv_handle_t* handle, void* arg)
{
/*
	zval retval = {0};
	zval params[2] = {0};
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	ZVAL_LONG(&params[0], status);
	ZVAL_OBJ(&params[1], &uv->std);

	php_uv_do_callback2(&retval, uv, params, 2, PHP_UV_PIPE_CONNECT_CB TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&retval);
	efree(req);
*/
}

static void php_uv_write_cb(uv_write_t* req, int status)
{
	write_req_t* wr = (write_req_t*) req;
	zval retval = {0};
	zval params[2] = {0};
	php_uv_t *uv = (php_uv_t *) req->handle->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	PHP_UV_DEBUG_PRINT("uv_write_cb: status: %d\n", status);

	ZVAL_OBJ(&params[0], &uv->std);
	ZVAL_LONG(&params[1], status);

	php_uv_do_callback(&retval, wr->cb, params, 2 TSRMLS_CC);

	if (EG(exception)) {
		uv_stop(uv->uv.handle.loop);
	}

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_write_cb, uv);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	zval_ptr_dtor(&retval);

	php_uv_free_write_req(wr);
}

static void php_uv_udp_send_cb(uv_udp_send_t* req, int status)
{
	send_req_t* wr = (send_req_t*) req;
	zval retval = {0};
	zval params[2] = {0};
	php_uv_t *uv = (php_uv_t *) req->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	ZVAL_OBJ(&params[0], &uv->std);
	ZVAL_LONG(&params[1], status);

	php_uv_do_callback2(&retval, uv, params, 2, PHP_UV_SEND_CB TSRMLS_CC);

	if (!uv_is_closing(&uv->uv.handle)) { /* send_cb is invoked *before* the handle is marked as inactive - uv_close() will thus *not* increment the refcount and we must then not delete the refcount here */
		PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_udp_send_cb, uv);
		zval_ptr_dtor(&params[0]);
	}
	zval_ptr_dtor(&params[1]);

	zval_ptr_dtor(&retval);

	if (wr->buf.base) {
		efree(wr->buf.base);
	}
	efree(wr);
}

static void php_uv_listen_cb(uv_stream_t* server, int status)
{
	zval retval = {0};
	zval params[2] = {0};
	php_uv_t *uv = (php_uv_t *) server->data;

	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	ZVAL_OBJ(&params[0], &uv->std);
	GC_ADDREF(&uv->std);
	PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(uv_listen_cb, uv);
	ZVAL_LONG(&params[1], status);

	php_uv_do_callback2(&retval, uv, params, 2, PHP_UV_LISTEN_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_listen_cb, uv);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	zval_ptr_dtor(&retval);
}

static void php_uv_shutdown_cb(uv_shutdown_t* handle, int status)
{
	zval retval = {0};
	zval params[2] = {0};
	php_uv_t *uv = (php_uv_t *) handle->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	ZVAL_OBJ(&params[0], &uv->std);
	ZVAL_LONG(&params[1], status);

	php_uv_do_callback2(&retval, uv, params, 2, PHP_UV_SHUTDOWN_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_shutdown_cb, uv);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	zval_ptr_dtor(&retval);
}

static void php_uv_read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf)
{
	zval retval = {0};
	zval params[2] = {0};
	php_uv_t *uv = (php_uv_t *) handle->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	PHP_UV_DEBUG_PRINT("uv_read_cb\n");

	ZVAL_OBJ(&params[0], &uv->std);
	if (nread > 0) { // uv disables itself when it reaches EOF/error
		GC_ADDREF(&uv->std);
		PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(uv_read_cb, uv);
	}

	if (nread > 0) {
		ZVAL_STRINGL(&params[1], buf->base, nread);
	} else {
		ZVAL_LONG(&params[1], nread);
	}

	php_uv_do_callback2(&retval, uv, params, 2, PHP_UV_READ_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_read_cb, uv);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	zval_ptr_dtor(&retval);

	if (buf->base) {
		efree(buf->base);
	}
}

/* unused
static void php_uv_read2_cb(uv_pipe_t* handle, ssize_t nread, uv_buf_t buf, uv_handle_type pending)
{
	zval retval = {0};
	zval params[3] = {0};
	php_uv_t *uv = (php_uv_t*)handle->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	PHP_UV_DEBUG_PRINT("uv_read2_cb\n");

	ZVAL_OBJ(&params[0], &uv->std);
	if (nread > 0) { // uv disables itself when it reaches EOF/error
		GC_ADDREF(&uv->std);
		PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(uv_read2_cb, uv);
	}
	if (nread > 0) {
		ZVAL_STRINGL(&params[1], buf.base,nread);
	} else {
		ZVAL_LONG(&params[1], nread);
	}
	ZVAL_LONG(&params[2], pending);

	php_uv_do_callback2(&retval, uv, params, 3, PHP_UV_READ2_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_read2_cb, uv);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);

	zval_ptr_dtor(&retval);

	if (buf.base) {
		efree(buf.base);
	}
}
*/

static void php_uv_prepare_cb(uv_prepare_t* handle)
{
	zval retval = {0};
	zval params[1];
	php_uv_t *uv = (php_uv_t*)handle->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	PHP_UV_DEBUG_PRINT("prepare_cb\n");

	ZVAL_OBJ(&params[0], &uv->std);
	GC_ADDREF(&uv->std);
	PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(uv_prepare_cb, uv);

	php_uv_do_callback2(&retval, uv, params, 1, PHP_UV_PREPARE_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_prepare_cb, uv);
	zval_ptr_dtor(&params[0]);

	zval_ptr_dtor(&retval);
}

static void php_uv_check_cb(uv_check_t* handle)
{
	zval retval = {0};
	zval params[1];
	php_uv_t *uv = (php_uv_t*)handle->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	PHP_UV_DEBUG_PRINT("check_cb\n");

	ZVAL_OBJ(&params[0], &uv->std);
	GC_ADDREF(&uv->std);
	PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(uv_check_cb, uv);

	php_uv_do_callback2(&retval, uv, params, 1, PHP_UV_CHECK_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_check_cb, uv);
	zval_ptr_dtor(&params[0]);

	zval_ptr_dtor(&retval);
}


static void php_uv_async_cb(uv_async_t* handle)
{
	zval retval = {0};
	zval params[1];
	php_uv_t *uv = (php_uv_t*)handle->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	PHP_UV_DEBUG_PRINT("async_cb\n");

	ZVAL_OBJ(&params[0], &uv->std);
	GC_ADDREF(&uv->std);
	PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(uv_async_cb, uv);

	php_uv_do_callback2(&retval, uv, params, 1, PHP_UV_ASYNC_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_async_cb, uv);
	zval_ptr_dtor(&params[0]);

	zval_ptr_dtor(&retval);
}

#if defined(ZTS) && PHP_VERSION_ID < 80000
static void php_uv_work_cb(uv_work_t* req)
{
	zval retval = {0};
	php_uv_t *uv = (php_uv_t*)req->data;

	uv = (php_uv_t*)req->data;

	PHP_UV_DEBUG_PRINT("work_cb\n");

	php_uv_do_callback3(&retval, uv, NULL, 0, PHP_UV_WORK_CB);
}

static void php_uv_after_work_cb(uv_work_t* req, int status)
{
	zval retval = {0};
	php_uv_t *uv = (php_uv_t*)req->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	uv = (php_uv_t*)req->data;

	PHP_UV_DEBUG_PRINT("after_work_cb\n");

	php_uv_do_callback2(&retval, uv, NULL, 0, PHP_UV_AFTER_WORK_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_after_work_cb, uv);

	/* as uv_cancel inside destruct_uv will return EBUSY here as were still in the work callback, but freeing is safe here */
	clean_uv_handle(uv); /* this avoids a cancel */
	OBJ_RELEASE(&uv->std);
}
#endif

static void php_uv_fs_cb(uv_fs_t* req)
{
	zval params[3] = {0};
	zval retval = {0};
	php_uv_t *uv = (php_uv_t*)req->data;
	int argc, i = 0;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	PHP_UV_DEBUG_PRINT("# php_uv_fs_cb %p\n", uv);

	if (PHP_UV_IS_DTORED(uv)) {
		uv_fs_req_cleanup(req);

		OBJ_RELEASE(&uv->std);
		return;
	}

	if (!Z_ISUNDEF(uv->fs_fd)) {
		ZVAL_COPY_VALUE(&params[0], &uv->fs_fd);
		ZVAL_UNDEF(&uv->fs_fd);
	}

	switch (uv->uv.fs.fs_type) {
		case UV_FS_SYMLINK:
		case UV_FS_LINK:
		case UV_FS_CHMOD:
		case UV_FS_RENAME:
		case UV_FS_UNLINK:
		case UV_FS_RMDIR:
		case UV_FS_MKDIR:
		case UV_FS_CLOSE:
		case UV_FS_CHOWN:
		case UV_FS_UTIME:
		case UV_FS_FUTIME:
			argc = 1;
			ZVAL_LONG(&params[0], uv->uv.fs.result);
			break;

		case UV_FS_FCHMOD:
		case UV_FS_FCHOWN:
		case UV_FS_FTRUNCATE:
		case UV_FS_FDATASYNC:
		case UV_FS_FSYNC:
			argc = 2;
			ZVAL_LONG(&params[1], uv->uv.fs.result);
			break;

		case UV_FS_OPEN:
			argc = 1;
			if (uv->uv.fs.result < 0) {
				ZVAL_LONG(&params[0], uv->uv.fs.result);
			} else {
				PHP_UV_FD_TO_ZVAL(&params[0], uv->uv.fs.result)
				PHP_UV_DEBUG_PRINT("Creating fs handle %p\n", Z_RES(params[0]));
			}
			break;

		case UV_FS_SCANDIR:
			argc = 2;
			if (uv->uv.fs.result < 0) { /* req->ptr may be NULL here, but uv_fs_scandir_next() knows to handle it */
				ZVAL_LONG(&params[0], uv->uv.fs.result);
			} else {
				uv_dirent_t dent;

				array_init(&params[0]);
				while (UV_EOF != uv_fs_scandir_next(req, &dent)) {
					add_next_index_string(&params[0], dent.name);
				}
			}
			break;

		case UV_FS_LSTAT:
		case UV_FS_STAT:
			argc = 1;
			if (req->ptr != NULL) {
				params[0] = php_uv_make_stat((const uv_stat_t *) req->ptr);
			} else {
				ZVAL_LONG(&params[0], uv->uv.fs.result);
			}
			break;
		case UV_FS_FSTAT:
			argc = 2;
			if (req->ptr != NULL) {
				params[1] = php_uv_make_stat((const uv_stat_t *) req->ptr);
			} else {
				ZVAL_LONG(&params[1], uv->uv.fs.result);
			}
			break;

		case UV_FS_READLINK:
			argc = 1;
			if (uv->uv.fs.result == 0) {
				ZVAL_STRING(&params[0], req->ptr);
			} else {
				ZVAL_LONG(&params[0], uv->uv.fs.result);
			}
			break;

		case UV_FS_READ:
			argc = 2;
			if (uv->uv.fs.result >= 0) {
				ZVAL_STRINGL(&params[1], uv->buffer, uv->uv.fs.result);
			} else {
				ZVAL_LONG(&params[1], uv->uv.fs.result);
			}
			efree(uv->buffer);
			break;

		case UV_FS_SENDFILE:
			argc = 2;
			ZVAL_LONG(&params[1], uv->uv.fs.result);
			break;

		case UV_FS_WRITE:
			argc = 2;
			ZVAL_LONG(&params[1], uv->uv.fs.result);
			efree(uv->buffer);
			break;

		case UV_FS_UNKNOWN:
		case UV_FS_CUSTOM:
		default:
			argc = 0;
			php_error_docref(NULL, E_ERROR, "type; %d does not support yet.", uv->uv.fs.fs_type);
			break;
	}

	php_uv_do_callback2(&retval, uv, params, argc, PHP_UV_FS_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_fs_cb, uv);
	for (i = 0; i < argc; i++) {
		zval_ptr_dtor(&params[i]);
	}

	zval_ptr_dtor(&retval);

	if (!Z_ISUNDEF(uv->fs_fd_alt)) {
		zval_ptr_dtor(&uv->fs_fd_alt);
		ZVAL_UNDEF(&uv->fs_fd_alt);
	}

	uv_fs_req_cleanup(req);

	clean_uv_handle(uv);
	OBJ_RELEASE(&uv->std);
}

static void php_uv_fs_event_cb(uv_fs_event_t* req, const char* filename, int events, int status)
{
	zval params[4] = {0};
	zval retval = {0};
	php_uv_t *uv = (php_uv_t*)req->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	PHP_UV_DEBUG_PRINT("fs_event_cb: %s, %d\n", filename, status);

	ZVAL_OBJ(&params[0], &uv->std);
	GC_ADDREF(&uv->std);
	PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(uv_fs_event_cb, uv);
	if (filename) {
		ZVAL_STRING(&params[1], filename);
	} else {
		ZVAL_NULL(&params[1]);
	}
	ZVAL_LONG(&params[2], events);
	ZVAL_LONG(&params[3], status);

	php_uv_do_callback2(&retval, uv, params, 4, PHP_UV_FS_EVENT_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_fs_event_cb, uv);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);
	zval_ptr_dtor(&params[3]);

	zval_ptr_dtor(&retval);
}

static zval php_uv_stat_to_zval(const uv_stat_t *stat)
{
	zval result = {0};
	array_init(&result);

	add_assoc_long_ex(&result, ZEND_STRL("dev"), stat->st_dev);
	add_assoc_long_ex(&result, ZEND_STRL("ino"), stat->st_ino);
	add_assoc_long_ex(&result, ZEND_STRL("mode"), stat->st_mode);
	add_assoc_long_ex(&result, ZEND_STRL("nlink"), stat->st_nlink);
	add_assoc_long_ex(&result, ZEND_STRL("uid"), stat->st_uid);
	add_assoc_long_ex(&result, ZEND_STRL("gid"), stat->st_gid);
	add_assoc_long_ex(&result, ZEND_STRL("rdev"), stat->st_rdev);
	add_assoc_long_ex(&result, ZEND_STRL("size"), stat->st_size);

#ifndef PHP_WIN32
	add_assoc_long_ex(&result, ZEND_STRL("blksize"), stat->st_blksize);
	add_assoc_long_ex(&result, ZEND_STRL("blocks"), stat->st_blocks);
#endif

	add_assoc_long_ex(&result, ZEND_STRL("atime"), stat->st_atim.tv_sec);
	add_assoc_long_ex(&result, ZEND_STRL("mtime"), stat->st_mtim.tv_sec);
	add_assoc_long_ex(&result, ZEND_STRL("ctime"), stat->st_ctim.tv_sec);

	return result;
}

static void php_uv_fs_poll_cb(uv_fs_poll_t* handle, int status, const uv_stat_t* prev, const uv_stat_t* curr)
{
	zval params[4] = {0};
	zval retval = {0};
	php_uv_t *uv = (php_uv_t*)handle->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	ZVAL_OBJ(&params[0], &uv->std);
	GC_ADDREF(&uv->std);
	PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(uv_fs_poll_cb, uv);
	ZVAL_LONG(&params[1], status);
	params[2] = php_uv_stat_to_zval(prev);
	params[3] = php_uv_stat_to_zval(curr);

	php_uv_do_callback2(&retval, uv, params, 4, PHP_UV_FS_POLL_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_fs_poll_cb, uv);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);
	zval_ptr_dtor(&params[3]);

	zval_ptr_dtor(&retval);
}

static void php_uv_poll_cb(uv_poll_t* handle, int status, int events)
{
	zval params[4] = {0};
	zval retval = {0};
	php_uv_t *uv = (php_uv_t*)handle->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	ZVAL_OBJ(&params[0], &uv->std);
	if (status == 0) {
		GC_ADDREF(&uv->std);
	}
	PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(uv_poll_cb, uv);
	ZVAL_LONG(&params[1], status);
	ZVAL_LONG(&params[2], events);
	if (!Z_ISUNDEF(uv->fs_fd)) {
		ZVAL_COPY(&params[3], &uv->fs_fd);
	} else {
		PHP_UV_FD_TO_ZVAL(&params[3], uv->sock);
	}

	php_uv_do_callback2(&retval, uv, params, 4, PHP_UV_POLL_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_poll_cb, uv);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);
	zval_ptr_dtor(&params[3]);

	zval_ptr_dtor(&retval);
}


static void php_uv_udp_recv_cb(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)
{
	/* TODO: is this correctly implmented? */
	zval retval = {0};
	zval params[3] = {0};
	php_uv_t *uv = (php_uv_t*)handle->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	ZVAL_OBJ(&params[0], &uv->std);
	GC_ADDREF(&uv->std);
	PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(uv_udp_recv_cb, uv);
	if (nread < 0) {
		ZVAL_LONG(&params[1], nread);
	} else {
		ZVAL_STRINGL(&params[1], buf->base, nread);
	}
	ZVAL_LONG(&params[2], flags);

	php_uv_do_callback2(&retval, uv, params, 3, PHP_UV_RECV_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_udp_recv_cb, uv);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);

	zval_ptr_dtor(&retval);

	if (buf->base) {
		efree(buf->base);
	}
}

static void php_uv_read_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	buf->base = emalloc(suggested_size);
	buf->len = suggested_size;
}

static void php_uv_close_cb(uv_handle_t *handle)
{
	zval retval = {0};
	zval params[1] = {0};

	php_uv_t *uv = (php_uv_t *) handle->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	if (uv->callback[PHP_UV_CLOSE_CB]) {
		ZVAL_OBJ(&params[0], &uv->std);

		php_uv_do_callback2(&retval, uv, params, 1, PHP_UV_CLOSE_CB TSRMLS_CC);
		zval_ptr_dtor(&retval);
	}

	/* manually clean the uv handle as dtor will not be called anymore here */
	clean_uv_handle(uv);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(uv_close_cb, uv);
	OBJ_RELEASE(&uv->std);
}

static inline zend_bool php_uv_is_handle_referenced(php_uv_t *uv) {
	zend_class_entry *ce = uv->std.ce;

	return (ce == uv_signal_ce || ce == uv_timer_ce || ce == uv_idle_ce || ce == uv_udp_ce || ce == uv_tcp_ce || ce == uv_tty_ce || ce == uv_pipe_ce || ce == uv_prepare_ce || ce == uv_check_ce || ce == uv_poll_ce || ce == uv_fs_poll_ce) && uv_is_active(&uv->uv.handle);
}

/* uv handle must not be cleaned or closed before called */
static void php_uv_close(php_uv_t *uv) {
	ZEND_ASSERT(!uv_is_closing(&uv->uv.handle));

	if (!php_uv_is_handle_referenced(uv)) {
		GC_ADDREF(&uv->std);
		PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(php_uv_close, uv);
	}

	uv_close(&uv->uv.handle, php_uv_close_cb);

	PHP_UV_SKIP_DTOR(uv);
}

static void php_uv_idle_cb(uv_timer_t *handle)
{
	zval retval = {0};
	zval params[1] = {0};

	php_uv_t *uv = (php_uv_t*)handle->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	ZVAL_OBJ(&params[0], &uv->std);
	GC_ADDREF(&uv->std);
	PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(php_uv_idle_cb, uv);

	php_uv_do_callback2(&retval, uv, params, 1, PHP_UV_IDLE_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(php_uv_idle_cb, uv);
	zval_ptr_dtor(&params[0]);

	zval_ptr_dtor(&retval);
}

static void php_uv_getaddrinfo_cb(uv_getaddrinfo_t* handle, int status, struct addrinfo* res)
{
	zval retval = {0};
	zval params[1] = {0};
	struct addrinfo *address;
	char ip[INET6_ADDRSTRLEN];
	const char *addr;

	php_uv_t *uv = (php_uv_t*)handle->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	if (status != 0) {
		ZVAL_LONG(&params[0], status);
	} else {
		array_init(&params[0]);

		address = res;
		while (address) {
			if (address->ai_family == AF_INET) {
				addr = (char*) &((struct sockaddr_in*) address->ai_addr)->sin_addr;
				uv_inet_ntop(address->ai_family, addr, ip, INET6_ADDRSTRLEN);
				add_next_index_string(&params[0], ip);
			}

			address = address->ai_next;
		}

		address = res;
		while (address) {
			if (address->ai_family == AF_INET6) {
				addr = (char*) &((struct sockaddr_in6*) address->ai_addr)->sin6_addr;
				uv_inet_ntop(address->ai_family, addr, ip, INET6_ADDRSTRLEN);
				add_next_index_string(&params[0], ip);
			}

			address = address->ai_next;
		}
	}

	php_uv_do_callback2(&retval, uv, params, 1, PHP_UV_GETADDR_CB TSRMLS_CC);

	zval_ptr_dtor(&retval);
	zval_ptr_dtor(&params[0]);

	uv_freeaddrinfo(res);
	clean_uv_handle(uv);
	OBJ_RELEASE(&uv->std);
}

static void php_uv_timer_cb(uv_timer_t *handle)
{
	zval retval = {0};
	zval params[1] = {0};
	php_uv_t *uv = (php_uv_t*)handle->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	ZVAL_OBJ(&params[0], &uv->std);

	if (handle->repeat) {
		GC_ADDREF(&uv->std);
		PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(php_uv_timer_cb, uv);
	}

	php_uv_do_callback2(&retval, uv, params, 1, PHP_UV_TIMER_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(php_uv_timer_cb, uv);
	zval_ptr_dtor(&params[0]);

	zval_ptr_dtor(&retval);
}

static void php_uv_signal_cb(uv_signal_t *handle, int sig_num)
{
	zval retval = {0};
	zval params[2] = {0};
	php_uv_t *uv = (php_uv_t*)handle->data;
	TSRMLS_FETCH_FROM_CTX(uv->thread_ctx);

	ZVAL_OBJ(&params[0], &uv->std);
	GC_ADDREF(&uv->std);
	PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(php_uv_signal_cb, uv);
	ZVAL_LONG(&params[1], sig_num);

	php_uv_do_callback2(&retval, uv, params, 2, PHP_UV_SIGNAL_CB TSRMLS_CC);

	PHP_UV_DEBUG_OBJ_DEL_REFCOUNT(php_uv_signal_cb, uv);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	zval_ptr_dtor(&retval);
}

void static destruct_uv_stdio(zend_object *obj)
{
	php_uv_stdio_t *stdio = (php_uv_stdio_t *) obj;

	zval_ptr_dtor(&stdio->stream);
}


/* common functions */

static void php_uv_ip_common(int ip_type, INTERNAL_FUNCTION_PARAMETERS)
{
	php_uv_sockaddr_t *addr;
	char ip[INET6_ADDRSTRLEN];

	ZEND_PARSE_PARAMETERS_START(1, 1)
		UV_PARAM_OBJ(addr, php_uv_sockaddr_t, (ip_type == 1) ? uv_sockaddr_ipv4_ce : uv_sockaddr_ipv6_ce)
	ZEND_PARSE_PARAMETERS_END();

	if (ip_type == 1) {
		uv_ip4_name(PHP_UV_SOCKADDR_IPV4_P(addr), ip, INET6_ADDRSTRLEN);
	} else {
		uv_ip6_name(PHP_UV_SOCKADDR_IPV6_P(addr), ip, INET6_ADDRSTRLEN);
	}
	RETVAL_STRING(ip);
}

static void php_uv_socket_bind(enum php_uv_socket_type ip_type, INTERNAL_FUNCTION_PARAMETERS)
{
	php_uv_sockaddr_t *addr;
	php_uv_t *uv;
	zend_long flags = 0;
	int r;

	if (ip_type & PHP_UV_UDP) {
		ZEND_PARSE_PARAMETERS_START(2, 3)
			UV_PARAM_OBJ(uv, php_uv_t, uv_udp_ce)
			UV_PARAM_OBJ(addr, php_uv_sockaddr_t, (ip_type == PHP_UV_UDP_IPV4) ? uv_sockaddr_ipv4_ce : uv_sockaddr_ipv6_ce)
			Z_PARAM_OPTIONAL
			Z_PARAM_LONG(flags)
		ZEND_PARSE_PARAMETERS_END();
	} else {
		ZEND_PARSE_PARAMETERS_START(2, 2)
			UV_PARAM_OBJ(uv, php_uv_t, uv_tcp_ce)
			UV_PARAM_OBJ(addr, php_uv_sockaddr_t, (ip_type == PHP_UV_TCP_IPV4) ? uv_sockaddr_ipv4_ce : uv_sockaddr_ipv6_ce)
		ZEND_PARSE_PARAMETERS_END();
	}

	switch (ip_type) {
		case PHP_UV_TCP_IPV4:
			r = uv_tcp_bind((uv_tcp_t*)&uv->uv.tcp, (const struct sockaddr*)&PHP_UV_SOCKADDR_IPV4(addr), 0);
			break;
		case PHP_UV_TCP_IPV6:
			r = uv_tcp_bind((uv_tcp_t*)&uv->uv.tcp, (const struct sockaddr*)&PHP_UV_SOCKADDR_IPV6(addr), 0);
			break;
		case PHP_UV_UDP_IPV4:
			r = uv_udp_bind((uv_udp_t*)&uv->uv.udp, (const struct sockaddr*)&PHP_UV_SOCKADDR_IPV4(addr), flags);
			break;
		case PHP_UV_UDP_IPV6:
			r = uv_udp_bind((uv_udp_t*)&uv->uv.udp, (const struct sockaddr*)&PHP_UV_SOCKADDR_IPV6(addr), flags);
			break;
		default:
			php_error_docref(NULL, E_ERROR, "unhandled type");
			return;
	}

	if (r) {
		php_error_docref(NULL, E_WARNING, "bind failed");
		RETURN_FALSE;
	} else {
		RETURN_TRUE;
	}
}

static void php_uv_socket_getname(int type, INTERNAL_FUNCTION_PARAMETERS)
{
	php_uv_t *uv;
	zval result;
	int addr_len;
	struct sockaddr_storage addr;
	addr_len = sizeof(struct sockaddr_storage);

	ZEND_PARSE_PARAMETERS_START(1, 1)
		UV_PARAM_OBJ(uv, php_uv_t, type == 3 ? uv_udp_ce : uv_tcp_ce)
	ZEND_PARSE_PARAMETERS_END();

	switch (type) {
		case 1:
			uv_tcp_getsockname(&uv->uv.tcp, (struct sockaddr*)&addr, &addr_len);
			break;
		case 2:
			uv_tcp_getpeername(&uv->uv.tcp, (struct sockaddr*)&addr, &addr_len);
			break;
		case 3:
			uv_udp_getsockname(&uv->uv.udp, (struct sockaddr*)&addr, &addr_len);
			break;
		default:
			php_error_docref(NULL, E_ERROR, "unexpected type");
		break;
	}

	result = php_uv_address_to_zval((struct sockaddr*)&addr);
	RETURN_ZVAL(&result, 0, 1);
}

static void php_uv_handle_open(int (*open_cb)(uv_handle_t *, long), zend_class_entry *ce, INTERNAL_FUNCTION_PARAMETERS) {
	php_uv_t *uv;
	zval *zstream;
	zend_long fd; // file handle
	int error;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		UV_PARAM_OBJ(uv, php_uv_t, ce)
		Z_PARAM_ZVAL(zstream)
	ZEND_PARSE_PARAMETERS_END();

	fd = php_uv_zval_to_fd(zstream);
	if (fd < 0) {
		php_error_docref(NULL, E_WARNING, "file descriptor must be unsigned value or a valid resource");
		RETURN_FALSE;
	}

	error = open_cb(&uv->uv.handle, fd);

	if (error) {
		php_error_docref(NULL, E_WARNING, "%s", php_uv_strerror(error));
	}

	RETURN_LONG(error);
}

static void php_uv_udp_send(int type, INTERNAL_FUNCTION_PARAMETERS)
{
	zend_string *data;
	php_uv_t *uv;
	send_req_t *w;
	php_uv_sockaddr_t *addr;
	zend_fcall_info fci       = empty_fcall_info;
	zend_fcall_info_cache fcc = empty_fcall_info_cache;
	php_uv_cb_t *cb;

	ZEND_PARSE_PARAMETERS_START(3, 4)
		UV_PARAM_OBJ(uv, php_uv_t, uv_udp_ce)
		Z_PARAM_STR(data)
		UV_PARAM_OBJ(addr, php_uv_sockaddr_t, (type == 1) ? uv_sockaddr_ipv4_ce : uv_sockaddr_ipv6_ce, uv_sockaddr_ipv6_ce)
		Z_PARAM_OPTIONAL
		Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	GC_ADDREF(&uv->std);
	PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(uv_udp_send, uv);

	PHP_UV_INIT_SEND_REQ(w, uv, data->val, data->len);
	php_uv_cb_init(&cb, uv, &fci, &fcc, PHP_UV_SEND_CB);

	if (addr->std.ce == uv_sockaddr_ipv4_ce) {
		uv_udp_send(&w->req, &uv->uv.udp, &w->buf, 1, (const struct sockaddr*)&PHP_UV_SOCKADDR_IPV4(addr), php_uv_udp_send_cb);
	} else {
		uv_udp_send(&w->req, &uv->uv.udp, &w->buf, 1, (const struct sockaddr*)&PHP_UV_SOCKADDR_IPV6(addr), php_uv_udp_send_cb);
	}
}

static void php_uv_tcp_connect(enum php_uv_socket_type type, INTERNAL_FUNCTION_PARAMETERS)
{
	php_uv_t *uv;
	php_uv_sockaddr_t *addr;
	uv_connect_t *req;
	zend_fcall_info fci       = empty_fcall_info;
	zend_fcall_info_cache fcc = empty_fcall_info_cache;
	php_uv_cb_t *cb;

	ZEND_PARSE_PARAMETERS_START(2, 3)
		UV_PARAM_OBJ(uv, php_uv_t, uv_tcp_ce)
		UV_PARAM_OBJ(addr, php_uv_sockaddr_t, (type == PHP_UV_TCP_IPV4) ? uv_sockaddr_ipv4_ce : uv_sockaddr_ipv6_ce, uv_sockaddr_ipv6_ce)
		Z_PARAM_OPTIONAL
		Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	GC_ADDREF(&uv->std);
	PHP_UV_DEBUG_OBJ_ADD_REFCOUNT(uv_tcp_connect, uv);

	PHP_UV_INIT_CONNECT(req, uv)
	php_uv_cb_init(&cb, uv, &fci, &fcc, PHP_UV_CONNECT_CB);

	if (addr->std.ce == uv_sockaddr_ipv4_ce) {
		uv_tcp_connect(req, &uv->uv.tcp, (const struct sockaddr*)&PHP_UV_SOCKADDR_IPV4(addr), php_uv_tcp_connect_cb);
	} else {
		uv_tcp_connect(req, &uv->uv.tcp,  (const struct sockaddr*)&PHP_UV_SOCKADDR_IPV6(addr), php_uv_tcp_connect_cb);
	}
}

/* zend */

static zend_function_entry php_uv_empty_methods[] = {
	{0}
};

#if PHP_VERSION_ID >= 80000
int php_uv_cast_object(zend_object *readobj, zval *writeobj, int type) {
#else
int php_uv_cast_object(zval *readobj_zv, zval *writeobj, int type) {
	zend_object *readobj = Z_OBJ_P(readobj_zv);
#endif
	if (type == IS_LONG) {
		ZVAL_LONG(writeobj, readobj->handle);
		return SUCCESS;
	} else {
#if PHP_VERSION_ID >= 80000
		return zend_std_cast_object_tostring(readobj, writeobj, type);
#else
		return zend_std_cast_object_tostring(readobj_zv, writeobj, type);
#endif
	}
}

#if PHP_VERSION_ID >= 80000
static HashTable *php_uv_get_debug_info(zend_object *object, int *is_temp) {
	php_uv_t *uv = (php_uv_t *) object;
#else
static HashTable *php_uv_get_debug_info(zval *object, int *is_temp) {
	php_uv_t *uv = (php_uv_t *) Z_OBJ_P(object);
#endif
	HashTable *ht = zend_std_get_debug_info(object, is_temp);
	if (uv->std.ce == uv_poll_ce) {
		if (!*is_temp) {
			int fd;
			if (uv_fileno(&uv->uv.handle, (uv_os_fd_t *)&fd) == 0) { /* not actually a fd on windows but a handle pointr address, but okay. */
				*is_temp = 1;
				ht = zend_array_dup(ht);
				zval fdzv;
				ZVAL_LONG(&fdzv, fd);
				zend_hash_update(ht, zend_string_init("@fd", sizeof("@fd")-1, 0), &fdzv);
			}
		}
	}
	return ht;
}

#if PHP_VERSION_ID >= 80000
static HashTable *php_uv_get_gc(zend_object *object, zval **table, int *n) {
	php_uv_t *uv = (php_uv_t *) object;
#else
static HashTable *php_uv_get_gc(zval *object, zval **table, int *n) {
	php_uv_t *uv = (php_uv_t *) Z_OBJ_P(object);
#endif
	int i;

	if (PHP_UV_IS_DTORED(uv)) {
		*n = 0;
		return NULL;
	}

	// include trailing zvals like fs_fd/_alt
	*n = (sizeof(php_uv_t) -  XtOffsetOf(php_uv_t, gc_data)) / sizeof(zval);
	for (i = 0; i < PHP_UV_CB_MAX; i++) {
		php_uv_cb_t *cb = uv->callback[i];
		if (cb) {
			ZVAL_COPY_VALUE(&uv->gc_data[i * 2], &cb->fci.function_name);
			if (cb->fci.object) {
				ZVAL_OBJ(&uv->gc_data[i * 2 + 1], cb->fci.object);
			}
		} else {
			ZVAL_UNDEF(&uv->gc_data[i * 2]);
			ZVAL_UNDEF(&uv->gc_data[i * 2 + 1]);
		}
	}
	*table = uv->gc_data;

	return uv->std.properties;
}

static void php_uv_loop_get_gc_walk_cb(uv_handle_t* handle, void *arg) {
	struct { int *n; php_uv_loop_t *loop; } *data = arg;
	php_uv_t *uv = (php_uv_t *) handle->data;

	if (php_uv_is_handle_referenced(uv)) {
		php_uv_loop_t *loop = data->loop;

		if (*data->n == loop->gc_buffer_size) {
			if (loop->gc_buffer_size == 0) {
				loop->gc_buffer_size = 16;
			} else {
				loop->gc_buffer_size *= 2;
			}
			loop->gc_buffer = erealloc(loop->gc_buffer, loop->gc_buffer_size * sizeof(zval));
		}

		ZVAL_OBJ(loop->gc_buffer + (*data->n)++, &uv->std);
	}
}


#if PHP_VERSION_ID >= 80000
static HashTable *php_uv_loop_get_gc(zend_object *object, zval **table, int *n) {
	php_uv_loop_t *loop = (php_uv_loop_t *) object;
#else
static HashTable *php_uv_loop_get_gc(zval *object, zval **table, int *n) {
	php_uv_loop_t *loop = (php_uv_loop_t *) Z_OBJ_P(object);
#endif
	struct { int *n; php_uv_loop_t *loop; } data;
	data.n = n;
	data.loop = loop;

	*n = 0;
	if (!PHP_UV_IS_DTORED(loop)) {
		uv_walk(&loop->loop, php_uv_loop_get_gc_walk_cb, &data);
		*table = loop->gc_buffer;
	}

	return loop->std.properties;
}

#if PHP_VERSION_ID >= 80000
static HashTable *php_uv_stdio_get_gc(zend_object *object, zval **table, int *n) {
	php_uv_stdio_t *stdio = (php_uv_stdio_t *) object;
#else
static HashTable *php_uv_stdio_get_gc(zval *object, zval **table, int *n) {
	php_uv_stdio_t *stdio = (php_uv_stdio_t *) Z_OBJ_P(object);
#endif

	*n = 1;
	*table = &stdio->stream;

	return stdio->std.properties;
}

static zend_object *php_uv_create_uv(zend_class_entry *ce) {
	php_uv_t *uv = emalloc(sizeof(php_uv_t));
	zend_object_std_init(&uv->std, ce);
	uv->std.handlers = &uv_handlers;

	PHP_UV_INIT_ZVALS(uv);
	TSRMLS_SET_CTX(uv->thread_ctx);

	uv->uv.handle.data = uv;

	return &uv->std;
}

static zend_object *php_uv_create_uv_loop(zend_class_entry *ce) {
	php_uv_loop_t *loop = emalloc(sizeof(php_uv_loop_t));
	zend_object_std_init(&loop->std, ce);
	loop->std.handlers = &uv_loop_handlers;
	
	uv_loop_init(&loop->loop);

	loop->gc_buffer_size = 0;
	loop->gc_buffer = NULL;

	return &loop->std;
}

static zend_object *php_uv_create_uv_sockaddr(zend_class_entry *ce) {
	php_uv_sockaddr_t *sockaddr = emalloc(sizeof(php_uv_sockaddr_t));
	zend_object_std_init(&sockaddr->std, ce);
	sockaddr->std.handlers = &uv_default_handlers;

	return &sockaddr->std;
}

static zend_object *php_uv_create_uv_lock(zend_class_entry *ce) {
	php_uv_lock_t *lock = emalloc(sizeof(php_uv_lock_t));
	zend_object_std_init(&lock->std, ce);
	lock->std.handlers = &uv_lock_handlers;

	lock->locked = 0;

	return &lock->std;
}

static zend_object *php_uv_create_uv_stdio(zend_class_entry *ce) {
	php_uv_stdio_t *stdio = emalloc(sizeof(php_uv_stdio_t));
	zend_object_std_init(&stdio->std, ce);
	stdio->std.handlers = &uv_stdio_handlers;

	stdio->flags = 0;
	ZVAL_UNDEF(&stdio->stream);

	return &stdio->std;
}

static zend_class_entry *php_uv_register_internal_class_ex(const char *name, zend_class_entry *parent) {
	zend_class_entry ce = {0}, *new;

	ce.name = zend_new_interned_string(zend_string_init(name, strlen(name), 1));
	ce.info.internal.builtin_functions = php_uv_empty_methods;
	new = zend_register_internal_class_ex(&ce, parent);
	new->serialize = zend_class_serialize_deny;
	new->unserialize = zend_class_unserialize_deny;
	new->ce_flags |= ZEND_ACC_FINAL;
	new->create_object = php_uv_create_uv;

	return new;
}

static zend_class_entry *php_uv_register_internal_class(const char *name) {
	return php_uv_register_internal_class_ex(name, NULL);
}

static zend_function *php_uv_get_ctor(zend_object *object) {
	zend_throw_error(NULL, "The UV classes cannot be instantiated manually");
	return NULL;
}

PHP_MINIT_FUNCTION(uv)
{
	PHP_UV_PROBE(MINIT);

#if PHP_VERSION_ID >= 70300
	memcpy(&uv_default_handlers, &std_object_handlers, sizeof(zend_object_handlers));
#else
	memcpy(&uv_default_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
#endif
	uv_default_handlers.clone_obj = NULL;
	uv_default_handlers.get_constructor = php_uv_get_ctor;
	uv_default_handlers.cast_object = php_uv_cast_object;

	uv_ce = php_uv_register_internal_class("UV");
	uv_ce->ce_flags |= ZEND_ACC_ABSTRACT;
	uv_ce->ce_flags &= ~ZEND_ACC_FINAL;
	memcpy(&uv_handlers, &uv_default_handlers, sizeof(zend_object_handlers));
	uv_handlers.get_gc = php_uv_get_gc;
	uv_handlers.dtor_obj = destruct_uv;
	uv_handlers.get_debug_info = php_uv_get_debug_info;

	php_uv_init(uv_ce);

	uv_stream_ce = php_uv_register_internal_class_ex("UVStream", uv_ce);
	uv_stream_ce->ce_flags |= ZEND_ACC_ABSTRACT;
	uv_stream_ce->ce_flags &= ~ZEND_ACC_FINAL;

	uv_tcp_ce = php_uv_register_internal_class_ex("UVTcp", uv_stream_ce);
	uv_udp_ce = php_uv_register_internal_class_ex("UVUdp", uv_ce);
	uv_pipe_ce = php_uv_register_internal_class_ex("UVPipe", uv_stream_ce);
	uv_idle_ce = php_uv_register_internal_class_ex("UVIdle", uv_ce);
	uv_timer_ce = php_uv_register_internal_class_ex("UVTimer", uv_ce);
	uv_async_ce = php_uv_register_internal_class_ex("UVAsync", uv_ce);
	uv_addrinfo_ce = php_uv_register_internal_class_ex("UVAddrinfo", uv_ce);
	uv_process_ce = php_uv_register_internal_class_ex("UVProcess", uv_ce);
	uv_prepare_ce = php_uv_register_internal_class_ex("UVPrepare", uv_ce);
	uv_check_ce = php_uv_register_internal_class_ex("UVCheck", uv_ce);
	uv_work_ce = php_uv_register_internal_class_ex("UVWork", uv_ce);
	uv_fs_ce = php_uv_register_internal_class_ex("UVFs", uv_ce);
	uv_fs_event_ce = php_uv_register_internal_class_ex("UVFsEvent", uv_ce);
	uv_tty_ce = php_uv_register_internal_class_ex("UVTty", uv_stream_ce);
	uv_fs_poll_ce = php_uv_register_internal_class_ex("UVFsPoll", uv_ce);
	uv_poll_ce = php_uv_register_internal_class_ex("UVPoll", uv_ce);
	uv_signal_ce = php_uv_register_internal_class_ex("UVSignal", uv_ce);

	uv_loop_ce = php_uv_register_internal_class("UVLoop");
	uv_loop_ce->create_object = php_uv_create_uv_loop;
	memcpy(&uv_loop_handlers, &uv_default_handlers, sizeof(zend_object_handlers));
	uv_loop_handlers.get_gc = php_uv_loop_get_gc;
	uv_loop_handlers.dtor_obj = destruct_uv_loop;

	uv_sockaddr_ce = php_uv_register_internal_class("UVSockAddr");
	uv_sockaddr_ce->ce_flags |= ZEND_ACC_ABSTRACT;
	uv_sockaddr_ce->ce_flags &= ~ZEND_ACC_FINAL;
	uv_sockaddr_ce->create_object = php_uv_create_uv_sockaddr;

	uv_sockaddr_ipv4_ce = php_uv_register_internal_class_ex("UVSockAddrIPv4", uv_sockaddr_ce);
	uv_sockaddr_ipv4_ce->create_object = php_uv_create_uv_sockaddr;

	uv_sockaddr_ipv6_ce = php_uv_register_internal_class_ex("UVSockAddrIPv6", uv_sockaddr_ce);
	uv_sockaddr_ipv6_ce->create_object = php_uv_create_uv_sockaddr;

	uv_lock_ce = php_uv_register_internal_class("UVLock");
	uv_lock_ce->create_object = php_uv_create_uv_lock;
	memcpy(&uv_lock_handlers, &uv_default_handlers, sizeof(zend_object_handlers));
	uv_lock_handlers.dtor_obj = destruct_uv_lock;

	uv_stdio_ce = php_uv_register_internal_class("UVStdio");
	uv_stdio_ce->create_object = php_uv_create_uv_stdio;
	memcpy(&uv_stdio_handlers, &uv_default_handlers, sizeof(zend_object_handlers));
	uv_stdio_handlers.dtor_obj = destruct_uv_stdio;
	uv_stdio_handlers.get_gc = php_uv_stdio_get_gc;

#if !defined(PHP_WIN32) && !(defined(HAVE_SOCKETS) && !defined(COMPILE_DL_SOCKETS))
	{
		zend_module_entry *sockets;
		if ((sockets = zend_hash_str_find_ptr(&module_registry, ZEND_STRL("sockets")))) {
			if (sockets->handle) { // shared
# if PHP_VERSION_ID >= 80000
				socket_ce = (int (*)(void)) DL_FETCH_SYMBOL(sockets->handle, "_socket_ce");
				if (socket_ce == NULL) {
					socket_ce = (int (*)(void)) DL_FETCH_SYMBOL(sockets->handle, "_socket_ce");
				}
# else
				php_sockets_le_socket = (int (*)(void)) DL_FETCH_SYMBOL(sockets->handle, "php_sockets_le_socket");
				if (php_sockets_le_socket == NULL) {
					php_sockets_le_socket = (int (*)(void)) DL_FETCH_SYMBOL(sockets->handle, "_php_sockets_le_socket");
				}
#endif
			}
		}
	}
#endif

	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(uv)
{
	if (UV_G(default_loop)) {
		uv_loop_t *loop = &UV_G(default_loop)->loop;

		/* for proper destruction: close all handles, let libuv call close callback and then close and free the loop */
		uv_stop(loop); /* in case we longjmp()'ed ... */
		uv_run(loop, UV_RUN_DEFAULT); /* invalidate the stop ;-) */

		uv_walk(loop, destruct_uv_loop_walk_cb, NULL);
		uv_run(loop, UV_RUN_DEFAULT);
		uv_loop_close(loop);
		OBJ_RELEASE(&UV_G(default_loop)->std);
	}

	return SUCCESS;
}





static zend_function_entry uv_functions[] = {
	/* general */
	PHP_FE(uv_update_time,              arginfo_uv_update_time)
	PHP_FE(uv_ref,                      arginfo_uv_ref)
	PHP_FE(uv_unref,                    arginfo_uv_unref)
	PHP_FE(uv_loop_new,                 NULL)
	PHP_FE(uv_default_loop,             NULL)
	PHP_FE(uv_stop,                     arginfo_uv_stop)
	PHP_FE(uv_run,                      arginfo_uv_run)
	PHP_FE(uv_ip4_addr,                 arginfo_uv_ip4_addr)
	PHP_FE(uv_ip6_addr,                 arginfo_uv_ip6_addr)
	PHP_FE(uv_ip4_name,                 arginfo_uv_ip4_name)
	PHP_FE(uv_ip6_name,                 arginfo_uv_ip6_name)
	PHP_FE(uv_write,                    arginfo_uv_write)
	PHP_FE(uv_write2,                   arginfo_uv_write2)
	PHP_FE(uv_shutdown,                 arginfo_uv_shutdown)
	PHP_FE(uv_close,                    arginfo_uv_close)
	PHP_FE(uv_now,                      arginfo_uv_now)
	PHP_FE(uv_loop_delete,              arginfo_uv_loop_delete)
	PHP_FE(uv_read_start,               arginfo_uv_read_start)
	PHP_FE(uv_read_stop,                arginfo_uv_read_stop)
	PHP_FE(uv_err_name,                 arginfo_uv_err_name)
	PHP_FE(uv_strerror,                 arginfo_uv_strerror)
	PHP_FE(uv_is_active,                arginfo_uv_is_active)
	PHP_FE(uv_is_closing,               arginfo_uv_is_closing)
	PHP_FE(uv_is_readable,              arginfo_uv_is_readable)
	PHP_FE(uv_is_writable,              arginfo_uv_is_writable)
	PHP_FE(uv_walk,                     arginfo_uv_walk)
	PHP_FE(uv_guess_handle,             arginfo_uv_guess_handle)
	/* idle */
	PHP_FE(uv_idle_init,                arginfo_uv_idle_init)
	PHP_FE(uv_idle_start,               arginfo_uv_idle_start)
	PHP_FE(uv_idle_stop,                arginfo_uv_idle_stop)
	/* timer */
	PHP_FE(uv_timer_init,               arginfo_uv_timer_init)
	PHP_FE(uv_timer_start,              arginfo_uv_timer_start)
	PHP_FE(uv_timer_stop,               arginfo_uv_timer_stop)
	PHP_FE(uv_timer_again,              arginfo_uv_timer_again)
	PHP_FE(uv_timer_set_repeat,         arginfo_uv_timer_set_repeat)
	PHP_FE(uv_timer_get_repeat,         arginfo_uv_timer_get_repeat)
	/* tcp */
	PHP_FE(uv_tcp_init,                 arginfo_uv_tcp_init)
	PHP_FE(uv_tcp_open,                 arginfo_uv_tcp_open)
	PHP_FE(uv_tcp_nodelay,              arginfo_uv_tcp_nodelay)
	PHP_FE(uv_tcp_bind,                 arginfo_uv_tcp_bind)
	PHP_FE(uv_tcp_bind6,                arginfo_uv_tcp_bind6)
	PHP_FE(uv_listen,                   arginfo_uv_listen)
	PHP_FE(uv_accept,                   arginfo_uv_accept)
	PHP_FE(uv_tcp_connect,              arginfo_uv_tcp_connect)
	PHP_FE(uv_tcp_connect6,             arginfo_uv_tcp_connect6)
	/* udp */
	PHP_FE(uv_udp_init,                 arginfo_uv_udp_init)
	PHP_FE(uv_udp_open,                 arginfo_uv_udp_open)
	PHP_FE(uv_udp_bind,                 arginfo_uv_udp_bind)
	PHP_FE(uv_udp_bind6,                arginfo_uv_udp_bind6)
	PHP_FE(uv_udp_set_multicast_loop,   arginfo_uv_udp_set_multicast_loop)
	PHP_FE(uv_udp_set_multicast_ttl,    arginfo_uv_udp_set_multicast_ttl)
	PHP_FE(uv_udp_send,                 arginfo_uv_udp_send)
	PHP_FE(uv_udp_send6,                arginfo_uv_udp_send6)
	PHP_FE(uv_udp_recv_start,           arginfo_uv_udp_recv_start)
	PHP_FE(uv_udp_recv_stop,            arginfo_uv_udp_recv_stop)
	PHP_FE(uv_udp_set_membership,       arginfo_uv_udp_set_membership)
	PHP_FE(uv_udp_set_broadcast,        arginfo_uv_udp_set_broadcast)
	/* poll */
	PHP_FE(uv_poll_init,                arginfo_uv_poll_init)
	PHP_FALIAS(uv_poll_init_socket,     uv_poll_init,  arginfo_uv_poll_init)
	PHP_FE(uv_poll_start,               arginfo_uv_poll_start)
	PHP_FE(uv_poll_stop,                arginfo_uv_poll_stop)
	PHP_FE(uv_fs_poll_init,             arginfo_uv_fs_poll_init)
	PHP_FE(uv_fs_poll_start,            arginfo_uv_fs_poll_start)
	PHP_FE(uv_fs_poll_stop,             arginfo_uv_fs_poll_stop)
	/* other network functions */
	PHP_FE(uv_tcp_getsockname,          arginfo_uv_tcp_getsockname)
	PHP_FE(uv_tcp_getpeername,          arginfo_uv_tcp_getpeername)
	PHP_FE(uv_udp_getsockname,          arginfo_uv_udp_getsockname)
	PHP_FE(uv_tcp_simultaneous_accepts, arginfo_uv_tcp_simultaneous_accepts)
	/* pipe */
	PHP_FE(uv_pipe_init,                arginfo_uv_pipe_init)
	PHP_FE(uv_pipe_bind,                arginfo_uv_pipe_bind)
	PHP_FE(uv_pipe_open,                arginfo_uv_pipe_open)
	PHP_FE(uv_pipe_connect,             arginfo_uv_pipe_connect)
	PHP_FE(uv_pipe_pending_instances,   arginfo_uv_pipe_pending_instances)
	PHP_FE(uv_pipe_pending_count,       arginfo_uv_pipe_pending_count)
	PHP_FE(uv_pipe_pending_type,        arginfo_uv_pipe_pending_type)
	PHP_FE(uv_stdio_new,                NULL)
	/* spawn */
	PHP_FE(uv_spawn,                    arginfo_uv_spawn)
	PHP_FE(uv_process_kill,             arginfo_uv_process_kill)
	PHP_FE(uv_process_get_pid,          arginfo_uv_process_get_pid)
	PHP_FE(uv_kill,                     arginfo_uv_kill)
	/* c-ares */
	PHP_FE(uv_getaddrinfo,              arginfo_uv_getaddrinfo)
	/* rwlock */
	PHP_FE(uv_rwlock_init,              NULL)
	PHP_FE(uv_rwlock_rdlock,            arginfo_uv_rwlock_rdlock)
	PHP_FE(uv_rwlock_tryrdlock,         arginfo_uv_rwlock_tryrdlock)
	PHP_FE(uv_rwlock_rdunlock,          arginfo_uv_rwlock_rdunlock)
	PHP_FE(uv_rwlock_wrlock,            arginfo_uv_rwlock_wrlock)
	PHP_FE(uv_rwlock_trywrlock,         arginfo_uv_rwlock_trywrlock)
	PHP_FE(uv_rwlock_wrunlock,          arginfo_uv_rwlock_wrunlock)
	/* mutex */
	PHP_FE(uv_mutex_init,               NULL)
	PHP_FE(uv_mutex_lock,               arginfo_uv_mutex_lock)
	PHP_FE(uv_mutex_trylock,            arginfo_uv_mutex_trylock)
	PHP_FE(uv_mutex_unlock,             arginfo_uv_mutex_unlock)
	/* semaphore */
	PHP_FE(uv_sem_init,                 arginfo_uv_sem_init)
	PHP_FE(uv_sem_post,                 arginfo_uv_sem_post)
	PHP_FE(uv_sem_wait,                 arginfo_uv_sem_wait)
	PHP_FE(uv_sem_trywait,              arginfo_uv_sem_trywait)
	/* prepare (before poll hook) */
	PHP_FE(uv_prepare_init,             arginfo_uv_prepare_init)
	PHP_FE(uv_prepare_start,            arginfo_uv_prepare_start)
	PHP_FE(uv_prepare_stop,             arginfo_uv_prepare_stop)
	/* check (after poll hook) */
	PHP_FE(uv_check_init,               arginfo_uv_check_init)
	PHP_FE(uv_check_start,              arginfo_uv_check_start)
	PHP_FE(uv_check_stop,               arginfo_uv_check_stop)
	/* async */
	PHP_FE(uv_async_init,               arginfo_uv_async_init)
	PHP_FE(uv_async_send,               arginfo_uv_async_send)
	/* queue (does not work yet) */
#if PHP_VERSION_ID < 80000
	PHP_FE(uv_queue_work,               NULL)
#endif
	/* fs */
	PHP_FE(uv_fs_open,                  arginfo_uv_fs_open)
	PHP_FE(uv_fs_read,                  arginfo_uv_fs_read)
	PHP_FE(uv_fs_write,                 arginfo_uv_fs_write)
	PHP_FE(uv_fs_close,                 arginfo_uv_fs_close)
	PHP_FE(uv_fs_fsync,                 arginfo_uv_fs_fsync)
	PHP_FE(uv_fs_fdatasync,             arginfo_uv_fs_fdatasync)
	PHP_FE(uv_fs_ftruncate,             arginfo_uv_fs_ftruncate)
	PHP_FE(uv_fs_mkdir,                 arginfo_uv_fs_mkdir)
	PHP_FE(uv_fs_rmdir,                 arginfo_uv_fs_rmdir)
	PHP_FE(uv_fs_unlink,                arginfo_uv_fs_unlink)
	PHP_FE(uv_fs_rename,                arginfo_uv_fs_rename)
	PHP_FE(uv_fs_utime,                 arginfo_uv_fs_utime)
	PHP_FE(uv_fs_futime,                arginfo_uv_fs_futime)
	PHP_FE(uv_fs_chmod,                 arginfo_uv_fs_chmod)
	PHP_FE(uv_fs_fchmod,                arginfo_uv_fs_fchmod)
	PHP_FE(uv_fs_chown,                 arginfo_uv_fs_chown)
	PHP_FE(uv_fs_fchown,                arginfo_uv_fs_fchown)
	PHP_FE(uv_fs_link,                  arginfo_uv_fs_link)
	PHP_FE(uv_fs_symlink,               arginfo_uv_fs_symlink)
	PHP_FE(uv_fs_readlink,              arginfo_uv_fs_readlink)
	PHP_FE(uv_fs_stat,                  arginfo_uv_fs_stat)
	PHP_FE(uv_fs_lstat,                 arginfo_uv_fs_lstat)
	PHP_FE(uv_fs_fstat,                 arginfo_uv_fs_fstat)
	PHP_FE(uv_fs_readdir,               arginfo_uv_fs_readdir)
	PHP_FE(uv_fs_scandir,               arginfo_uv_fs_scandir)
	PHP_FE(uv_fs_sendfile,              arginfo_uv_fs_sendfile)
	PHP_FE(uv_fs_event_init,            arginfo_uv_fs_event_init)
	/* tty */
	PHP_FE(uv_tty_init,                 arginfo_uv_tty_init)
	PHP_FE(uv_tty_get_winsize,          arginfo_uv_tty_get_winsize)
	PHP_FE(uv_tty_set_mode,             NULL)
	PHP_FE(uv_tty_reset_mode,           NULL)
	/* info */
	PHP_FE(uv_loadavg,                  NULL)
	PHP_FE(uv_uptime,                   NULL)
	PHP_FE(uv_cpu_info,                 NULL)
	PHP_FE(uv_interface_addresses,      NULL)
	PHP_FE(uv_get_free_memory,          NULL)
	PHP_FE(uv_get_total_memory,         NULL)
	PHP_FE(uv_hrtime,                   NULL)
	PHP_FE(uv_exepath,                  NULL)
	PHP_FE(uv_cwd,                      NULL)
	PHP_FE(uv_chdir,                    arginfo_uv_chdir)
	PHP_FE(uv_resident_set_memory,      NULL)
	/* signal handling */
	PHP_FE(uv_signal_init,              arginfo_uv_signal_init)
	PHP_FE(uv_signal_start,             arginfo_uv_signal_start)
	PHP_FE(uv_signal_stop,              arginfo_uv_signal_stop)
	{0}
};

PHP_MINFO_FUNCTION(uv)
{
	char uv_version[20];

	sprintf(uv_version, "%d.%d",UV_VERSION_MAJOR, UV_VERSION_MINOR);

	php_printf("PHP libuv Extension\n");
	php_info_print_table_start();
	php_info_print_table_header(2,"libuv Support",  "enabled");
	php_info_print_table_row(2,"Version", PHP_UV_VERSION);
	php_info_print_table_row(2,"libuv Version", uv_version);
	php_info_print_table_end();
}

static PHP_GINIT_FUNCTION(uv)
{
#if defined(COMPILE_DL_UV) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	uv_globals->default_loop = NULL;
}

zend_module_entry uv_module_entry = {
	STANDARD_MODULE_HEADER,
	"uv",
	uv_functions,					/* Functions */
	PHP_MINIT(uv),	/* MINIT */
	NULL,					/* MSHUTDOWN */
	NULL,					/* RINIT */
	PHP_RSHUTDOWN(uv),		/* RSHUTDOWN */
	PHP_MINFO(uv),	/* MINFO */
	PHP_UV_VERSION,
	PHP_MODULE_GLOBALS(uv),
	PHP_GINIT(uv),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};


#ifdef COMPILE_DL_UV
ZEND_GET_MODULE(uv)
#endif
