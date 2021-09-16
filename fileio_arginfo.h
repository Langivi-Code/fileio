/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 7eb3fd4083c98e6dffc8b02b6373b7ce9cbf228d */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_use_promise, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_enable_event, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_setTimeout, 0, 2, IS_NULL, 0)
                ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE_CHECK_SYNTAX_ONLY, 0)
                ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_idle, 0, 2, IS_NULL, 0)
                ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE_CHECK_SYNTAX_ONLY, 0)
ZEND_END_ARG_INFO()
//
//ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_file_get_contents_async, 0, 2, _IS_BOOL, 0)
//	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
//	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
//	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, useIncludePath, _IS_BOOL, 0, "false")
//	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, zcontext, IS_RESOURCE, 1, "null")
//	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
//	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, maxlen, IS_LONG, 0, "0")
//ZEND_END_ARG_INFO()


ZEND_FUNCTION(use_promise);
ZEND_FUNCTION(enable_event);
ZEND_FUNCTION(setTimeout);
ZEND_FUNCTION(idle);
//ZEND_FUNCTION(file_get_contents_async);


static const zend_function_entry file_io_functions[] = {
        ZEND_FE(use_promise, arginfo_use_promise)
        ZEND_FE(setTimeout, arginfo_setTimeout)
        ZEND_FE(idle, arginfo_idle)
        ZEND_FE(enable_event, arginfo_enable_event)
//	    ZEND_FE(file_get_contents_async, arginfo_file_get_contents_async)
        ZEND_FE_END
};
