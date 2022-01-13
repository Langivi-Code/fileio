/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 7eb3fd4083c98e6dffc8b02b6373b7ce9cbf228d */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_use_promise, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_enable_event, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_setTimeout, 0, 2, IS_LONG, 0)
                ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE_CHECK_SYNTAX_ONLY, 0)
                ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_clearTimeout, 0, 1, _IS_BOOL, 0)
                ZEND_ARG_TYPE_INFO(0, timerId, IS_LONG, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_setInterval, 0, 2, IS_LONG, 0)
                ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE_CHECK_SYNTAX_ONLY, 0)
                ZEND_ARG_TYPE_INFO(0, interval, IS_LONG, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_clearInterval, 0, 1, _IS_BOOL, 0)
                ZEND_ARG_TYPE_INFO(0, timerId, IS_LONG, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_idle, 0, 2, IS_NULL, 0)
                ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE_CHECK_SYNTAX_ONLY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_get_contents_async, 0, 2, _IS_BOOL, 0)
                ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
                ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
                ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
                ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, maxlen, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_put_contents_async, 0, 2, _IS_BOOL, 0)
                ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
                ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
                ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
                ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mysqla_query, 0, 2, IS_MIXED, 0)
                ZEND_ARG_TYPE_INFO(0, db, IS_MIXED, 0)
                ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()


ZEND_FUNCTION(use_promise);

ZEND_FUNCTION(enable_event);

ZEND_FUNCTION(set_timeout);

ZEND_FUNCTION(set_interval);

ZEND_FUNCTION(clear_timeout);

ZEND_FUNCTION(clear_interval);

ZEND_FUNCTION(idle);

ZEND_FUNCTION(file_get_contents_async);

ZEND_FUNCTION(file_put_contents_async);

PHP_FUNCTION (send_header);

ZEND_FUNCTION(mysqli_wait);
ZEND_FUNCTION(pg_wait);

static const zend_function_entry file_io_functions[] = {
        ZEND_FE(use_promise, arginfo_use_promise)
        ZEND_FE(set_timeout, arginfo_setTimeout)
        ZEND_FE(clear_timeout, arginfo_clearTimeout)
        ZEND_FE(set_interval, arginfo_setInterval)
        ZEND_FE(clear_interval, arginfo_clearInterval)
        ZEND_FE(idle, arginfo_idle)
        ZEND_FE(enable_event, arginfo_enable_event)
        ZEND_FE(file_get_contents_async, arginfo_file_get_contents_async)
        ZEND_FE(file_put_contents_async, arginfo_file_put_contents_async)
        ZEND_FE(mysqli_wait, arginfo_mysqla_query)
        ZEND_FE(pg_wait, arginfo_mysqla_query)
        ZEND_FE_END
};
