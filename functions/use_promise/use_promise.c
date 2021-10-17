//
// Created by admin on 14.09.2021.
//
#include <php.h>
#include <php_ini.h>
#include "zend.h"
#include <zend_API.h>
#include <zend_closures.h>
#include "../common/callback_interface.h"
#include "../../constants.h"
#include "../../php_fileio.h"
#include "../idle/idle_interface.h"
#include "zend_enum.h"
#include "promise.h"

zend_function *promise_resolve;
zend_function *promise_reject;
static const zend_function_entry enum_PromiseStatus_methods[] = {
        ZEND_FE_END
};


//static zend_object_handlers enum_handlers;
//zend_object *zend_enum_new(zval *result, zend_class_entry *ce, zend_string *case_name, zval *backing_value_zv)
//{
//    zend_object *zobj = zend_objects_new(ce);
//    ZVAL_OBJ(result, zobj);
//
//    ZVAL_STR_COPY(OBJ_PROP_NUM(zobj, 0), case_name);
//    if (backing_value_zv != NULL) {
//        ZVAL_COPY(OBJ_PROP_NUM(zobj, 1), backing_value_zv);
//    }
//
//    zobj->handlers = &enum_handlers;
//
//    return zobj;
//}

PHP_FUNCTION (use_promise) {
    printf("length %d", status_f.length);
//    zval property___status_default_value;
//    zval back;
//    ZVAL_NULL(&back);
//    zend_string * str= zend_string_init("Pending", sizeof("Pending") - 1, 1);
//        zend_enum_new(&property___status_default_value, promise_enum, str, &back);
//    RETURN_OBJ(&property___status_default_value);
    RETURN_BOOL(INI_BOOL("file_io.use_promise"));
}


zend_class_entry *create_PromiseStatus_enum(void) {
    FILE_IO_GLOBAL(promise__status_enum) = zend_register_internal_enum("PromiseStatus", IS_UNDEF,
                                                                       enum_PromiseStatus_methods);
    zend_enum_add_case_cstr(FILE_IO_GLOBAL(promise__status_enum), "Pending", NULL);
    zend_enum_add_case_cstr(FILE_IO_GLOBAL(promise__status_enum), "Resolved", NULL);
    zend_enum_add_case_cstr(FILE_IO_GLOBAL(promise__status_enum), "Rejected", NULL);
    return FILE_IO_GLOBAL(promise__status_enum);
}

//
//static zend_class_entry *promise_class_entry = NULL;
//
PHP_METHOD (Promise, __construct) {
    zval * callback;
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(callback)ZEND_PARSE_PARAMETERS_END();
    zend_update_property(FILE_IO_GLOBAL(promise_class), Z_OBJ_P(ZEND_THIS), "closure", sizeof("closure") - 1, callback);
    zend_fcall_info_init(callback, 0, &fci, &fcc, NULL, NULL);
//    printf("%p, %p, %p\n", rejected, rejecte2d, rejecte3d);
    zval * params = emalloc(2 * sizeof(zval));
    zval func;
    zval retavl;
    fci.retval = &retavl;
    zval paramsName;
    ZVAL_STRING(&paramsName, "trolo");
    //TODO fill params
    fci.param_count = 2;
    zend_create_fake_closure(&func, promise_resolve, FILE_IO_GLOBAL(promise_class), FILE_IO_GLOBAL(promise_class),
                             ZEND_THIS);
    ZVAL_COPY(&params[0], &func);
    zend_create_fake_closure(&func, promise_reject, FILE_IO_GLOBAL(promise_class), FILE_IO_GLOBAL(promise_class),
                             ZEND_THIS);
    ZVAL_COPY(&params[1], &func);
    fci.params = params;
#define LOG_TAG "PROMISE"
    uv_idle_t *idleHandle = emalloc(sizeof(uv_idle_t));

    uv_idle_init(FILE_IO_GLOBAL(loop), idleHandle);
    fill_idle_handle_with_data(idleHandle, &fci, &fcc);
    LOG("Setting idle ...\n");
    uv_idle_start(idleHandle, fn_idle);
    zval * status = emalloc(sizeof(zval));
    zend_object * pending = zend_enum_get_case_cstr(FILE_IO_GLOBAL(promise__status_enum), "Pending");
    ZVAL_OBJ(status, pending);
    zend_update_property(FILE_IO_GLOBAL(promise_class), Z_OBJ_P(ZEND_THIS), status_f.name, status_f.length, status);
//    uv_cb_type * cbs = emalloc(sizeof(uv_cb_type));
//    memcpy(&cbs->fci, &fci, sizeof(zend_fcall_info));
//    memcpy(&cbs->fcc, &fcc, sizeof(zend_fcall_info_cache));

//    printf("not found %d\n", NULL == promise_resolve);
//    zval callable3;
//    zval * callable4;
//    zend_call_known_function(promise_resolve, NULL, FILE_IO_GLOBAL(promise_class), &callable3, 0, NULL, NULL);
//    zend_update_property()
//  $this->handle();
// //        $this->internalFiber = new Fiber([$this, 'handle']);
// //        $this->status = $this->internalFiber->start();
//
}


PHP_METHOD (Promise, resolve) {
    printf("it resolved\n");
    zval * param;
    zval * status = emalloc(sizeof(zval));
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL_OR_NULL(param)ZEND_PARSE_PARAMETERS_END();
    zend_object * resolved = zend_enum_get_case_cstr(FILE_IO_GLOBAL(promise__status_enum), "Resolved");
    ZVAL_OBJ(status, resolved);
    zend_update_property(FILE_IO_GLOBAL(promise_class), Z_OBJ_P(ZEND_THIS), dataStore_f.name, dataStore_f.length,
                         param);

    zend_update_property(FILE_IO_GLOBAL(promise_class), Z_OBJ_P(ZEND_THIS), status_f.name, status_f.length, status);
    switch (Z_TYPE_P(param)) {
        case IS_STRING:
            php_printf("resolved %s val", ZSTR_VAL(Z_STR_P(param)));
            break;
        case IS_NULL:
            break;

    }
    RETURN_OBJ(resolved);
//         return new Promise(function ($res, $rej) use ($data) {
//             $res($data);
//         });
}

PHP_METHOD (Promise, reject) {
    printf("it rejected\n");
    zval * param;
    zval * status = emalloc(sizeof(zval));
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL_OR_NULL(param)ZEND_PARSE_PARAMETERS_END();

    zend_object * rejecte3d = zend_enum_get_case_cstr(FILE_IO_GLOBAL(promise__status_enum), "Rejected");
    zend_object * rejected = zend_enum_get_case_cstr(FILE_IO_GLOBAL(promise__status_enum), "Rejected");

    ZVAL_OBJ(status, rejected);
    zend_update_property(FILE_IO_GLOBAL(promise_class), Z_OBJ_P(ZEND_THIS), dataStore_f.name, dataStore_f.length,
                         param);
    zval rv;
    zend_update_property(FILE_IO_GLOBAL(promise_class), Z_OBJ_P(ZEND_THIS), status_f.name, status_f.length, status);
    zend_read_property(FILE_IO_GLOBAL(promise_class), Z_OBJ_P(ZEND_THIS), status_f.name, status_f.length, 0, &rv);
    printf("%p, %p,\n", rejected, &Z_OBJ(rv));
    switch (Z_TYPE_P(param)) {
        case IS_STRING:
            php_printf("rejected with  %s val", ZSTR_VAL(Z_STR_P(param)));
            break;
        case IS_NULL:
            break;

    }
    RETURN_OBJ(rejected);
}

PHP_METHOD (Promise, then) {
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_FUNC(fci, fcc)ZEND_PARSE_PARAMETERS_END();
    zval * data = emalloc(sizeof(zval));
    zval promiseFinalized;
    zval  retval;
    zend_long error;
    zend_read_property(FILE_IO_GLOBAL(promise_class), Z_OBJ_P(ZEND_THIS), promiseFinalized_f.name, promiseFinalized_f.length, 0, &promiseFinalized);
    zend_read_property(FILE_IO_GLOBAL(promise_class), Z_OBJ_P(ZEND_THIS), dataStore_f.name, dataStore_f.length, 0, data);
    fci.param_count = 1;
    fci.params = data;
    fci.retval = &retval;
    if (ZEND_FCI_INITIALIZED(fci)) {
        LOG("Then call back is called");
        if (zend_call_function(&fci, &fcc) != SUCCESS) {
            error = -1;
        }
    } else {
        error = -2;
    }
//    zend_object * resolved = zend_enum_get_case_cstr(FILE_IO_GLOBAL(promise__status_enum), "Resolved");

//    return new Promise(function ($res, $rej) use ($data) {
//             $rej($data);
//         });
}

PHP_METHOD (Promise, catch) {
//    return new Promise(function ($res, $rej) use ($data) {
//             $rej($data);
//         });
}

PHP_METHOD (Promise, finally) {
//    return new Promise(function ($res, $rej) use ($data) {
//             $rej($data);
//         });
}


//
ZEND_BEGIN_ARG_INFO(arginfo_promise_construct, 0)
                ZEND_ARG_TYPE_INFO(0, closure, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(arginfo_promise_resolve, 0)
                ZEND_ARG_TYPE_INFO(0, data, IS_MIXED, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(arginfo_promise_reject, 0)
                ZEND_ARG_TYPE_INFO(0, reason, IS_MIXED, 1)
ZEND_END_ARG_INFO()


static const zend_function_entry class_Promise_methods[] = {
        ZEND_ME(Promise, __construct, arginfo_promise_construct, ZEND_ACC_PUBLIC)
        ZEND_ME(Promise, resolve, arginfo_promise_resolve, ZEND_ACC_PUBLIC)
        ZEND_ME(Promise, reject, arginfo_promise_reject, ZEND_ACC_PUBLIC)
        ZEND_ME(Promise, then, arginfo_promise_construct, ZEND_ACC_PUBLIC)
        ZEND_ME(Promise, catch, arginfo_promise_construct, ZEND_ACC_PUBLIC)
        ZEND_ME(Promise, finally, arginfo_promise_construct, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

zend_class_entry *register_class_Promise(void) {
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Promise", class_Promise_methods);
    FILE_IO_GLOBAL(promise_class) = zend_register_internal_class_ex(&ce, NULL);


//    zend_declare_class_constant_long(promise_class_entry,
//                                     "DEFAULT_FACTOR", sizeof("DEFAULT_FACTOR")-1, DEFAULT_SCALE_FACTOR);
//    zend_declare_property_long(promise_class_entry,
//                               "status", sizeof("status")-1, DEFAULT_SCALE_FACTOR, ZEND_ACC_PRIVATE);
//    zval property_service_default_value;
//    ZVAL_NULL(&property_service_default_value);
//    zend_string *property_service_name = zend_string_init("service", sizeof("service") - 1, 1);
//    zend_declare_property_ex(class_entry, property_service_name, &property_service_default_value, ZEND_ACC_PRIVATE, NULL);
//    zend_string_release(property_service_name);

    zend_string * property__status_class_PromiseStatus = zend_string_init("PromiseStatus", sizeof("PromiseStatus") - 1,
                                                                          1);
    zval property___status_default_value;
    ZVAL_NULL(&property___status_default_value);
//    zend_enum_new(&property___status_default_value, promise_enum,
//                  zend_string_init("Pending", sizeof("Pending") - 1, 1), NULL);
    zend_string * property___status = zend_string_init("status", sizeof("status") - 1, 1);
    zend_declare_typed_property(FILE_IO_GLOBAL(promise_class), property___status, &property___status_default_value,
                                ZEND_ACC_PRIVATE, NULL,
                                (zend_type) ZEND_TYPE_INIT_CLASS(property__status_class_PromiseStatus, 0, MAY_BE_NULL));

//    zend_declare_typed_property(class_entry, property___status, &property___status_default_value,
//                                ZEND_ACC_PRIVATE, NULL,
//                                (zend_type) ZEND_TYPE_INIT_CLASS(property__status_class_PromiseStatus, 0, MAY_BE_NULL));
    zend_declare_property_long(FILE_IO_GLOBAL(promise_class),
                               "status", sizeof("status") - 1, 0, ZEND_ACC_PRIVATE);
    zend_declare_property_bool(FILE_IO_GLOBAL(promise_class), promiseFinalized_f.name, promiseFinalized_f.length, 0,
                               ZEND_ACC_PRIVATE);

    /**   $dataStore  **/
    zval property___dataStore_default_value;
    ZVAL_NULL(&property___dataStore_default_value);
    zend_string * property_dt_store_name = zend_string_init("dataStore", sizeof("dataStore") - 1, 1);
    zend_declare_property(FILE_IO_GLOBAL(promise_class), "dataStore", sizeof("dataStore") - 1,
                          &property___dataStore_default_value, ZEND_ACC_PRIVATE);
    zend_string_release(property_dt_store_name);

    /**   $closure  **/
    zval prop___closure_default_value;
    ZVAL_NULL(&prop___closure_default_value);
    zend_string * prop__closure_class_Closure = zend_string_init("Closure", sizeof("Closure") - 1, 1);
    zend_string * prop__closure_name = zend_string_init("closure", sizeof("closure") - 1,
                                                        1);
    zend_declare_typed_property(FILE_IO_GLOBAL(promise_class), prop__closure_name, &prop___closure_default_value,
                                ZEND_ACC_PUBLIC, NULL,
                                (zend_type) ZEND_TYPE_INIT_CLASS(prop__closure_class_Closure, 0, MAY_BE_CALLABLE));
    zend_string_release(prop__closure_name);

    return FILE_IO_GLOBAL(promise_class);
}
