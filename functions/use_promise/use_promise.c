//
// Created by admin on 14.09.2021.
//
#include <php.h>
#include <php_ini.h>
#include "zend.h"
#include <zend_API.h>
#include "zend_enum.h"
zend_class_entry *promise_enum;

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
//    zval property___status_default_value;
//    zval back;
//    ZVAL_NULL(&back);
//    zend_string * str= zend_string_init("Pending", sizeof("Pending") - 1, 1);
//        zend_enum_new(&property___status_default_value, promise_enum, str, &back);
//    RETURN_OBJ(&property___status_default_value);
    RETURN_BOOL(INI_BOOL("file_io.use_promise"));
}



zend_class_entry *create_PromiseStatus_enum(void) {
    zend_class_entry * promise_enum = zend_register_internal_enum("PromiseStatus", IS_UNDEF,
                                                                  enum_PromiseStatus_methods);
    zend_enum_add_case_cstr(promise_enum, "Pending", NULL);
    zend_enum_add_case_cstr(promise_enum, "Resolved", NULL);
    zend_enum_add_case_cstr(promise_enum, "Rejected", NULL);
    return promise_enum;
}
//
//static zend_class_entry *promise_class_entry = NULL;
//
PHP_METHOD (Promise, __construct) {
//  $this->handle();
// //        $this->internalFiber = new Fiber([$this, 'handle']);
// //        $this->status = $this->internalFiber->start();
//
}


PHP_METHOD (Promise, resolve) {
//         return new Promise(function ($res, $rej) use ($data) {
//             $res($data);
//         });
}

PHP_METHOD (Promise, reject) {
//    return new Promise(function ($res, $rej) use ($data) {
//             $rej($data);
//         });
}

PHP_METHOD (Promise, then) {
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
                ZEND_ARG_TYPE_INFO(0, data, IS_MIXED, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(arginfo_promise_reject, 0)
                ZEND_ARG_TYPE_INFO(0, reason, IS_MIXED, 0)
ZEND_END_ARG_INFO()
//
//ZEND_BEGIN_ARG_INFO(arginfo_scaler_scale, 0)
//                ZEND_ARG_INFO(1, x) // pass by reference
//ZEND_END_ARG_INFO()
//
//
static const zend_function_entry class_Promise_methods[] = {
        ZEND_ME(Promise, __construct, arginfo_promise_construct, ZEND_ACC_PUBLIC)
        ZEND_ME(Promise, resolve, arginfo_promise_resolve, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        ZEND_ME(Promise, reject, arginfo_promise_reject, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        ZEND_ME(Promise, then, arginfo_promise_construct, ZEND_ACC_PUBLIC)
        ZEND_ME(Promise, catch, arginfo_promise_construct, ZEND_ACC_PUBLIC)
        ZEND_ME(Promise, finally, arginfo_promise_construct, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

zend_class_entry *register_class_Promise(void) {
    zend_class_entry ce, *class_entry;

    INIT_CLASS_ENTRY(ce, "Promise", class_Promise_methods);
    class_entry = zend_register_internal_class_ex(&ce, NULL);

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
    zend_declare_typed_property(class_entry, property___status, &property___status_default_value,
                                ZEND_ACC_PRIVATE, NULL,
                                (zend_type) ZEND_TYPE_INIT_CLASS(property__status_class_PromiseStatus, 0, MAY_BE_NULL));
    zend_declare_typed_property(class_entry, property___status, &property___status_default_value,
                                ZEND_ACC_PRIVATE, NULL,
                                (zend_type) ZEND_TYPE_INIT_CLASS(property__status_class_PromiseStatus, 0, MAY_BE_NULL));
    zend_declare_property_long(class_entry,
                               "status", sizeof("status") - 1, 0, ZEND_ACC_PRIVATE);
    zend_declare_property_bool(class_entry, "promiseFinalised", sizeof("promiseFinalised") - 1, 0, ZEND_ACC_PRIVATE);
    zval property___dataStore_default_value;
    ZVAL_NULL(&property___dataStore_default_value);
    zend_declare_property(class_entry, "dataStore", sizeof("dataStore") - 1, &property___dataStore_default_value,
                          ZEND_ACC_PRIVATE);
    zend_string_release(property___status);

    return class_entry;
}
