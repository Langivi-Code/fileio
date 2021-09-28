//
// Created by admin on 14.09.2021.
//
#include <php.h>
#include <php_ini.h>
#include "zend_API.h"

PHP_FUNCTION (use_promise) {
    RETURN_BOOL(INI_BOOL("file_io.use_promise"));
}
//
//static zend_class_entry *promise_class_entry = NULL;
//
//PHP_METHOD (Promise, __construct) {
//
//}
//
//
//PHP_METHOD (Promise, resolve) {
//
//}
//
//ZEND_BEGIN_ARG_INFO(arginfo_promise_construct, 0)
//                ZEND_ARG_TYPE_INFO(0, closure, IS_CALLABLE, 0)
//ZEND_END_ARG_INFO()
//
//ZEND_BEGIN_ARG_INFO(arginfo_scaler_scale, 0)
//                ZEND_ARG_INFO(1, x) // pass by reference
//ZEND_END_ARG_INFO()
//
//
//static const zend_function_entry class_Promise_methods[] = {
//        ZEND_ME(Promise, __construct, arginfo_promise_construct, ZEND_ACC_PUBLIC)
//        ZEND_ME(Promise, resolve, arginfo_scaler_scale, ZEND_ACC_PUBLIC)
//        PHP_FE_END
//};
//
//static zend_class_entry *register_class_Promise(void)
//{
//    zend_class_entry ce, *class_entry;
//
//    INIT_CLASS_ENTRY(ce, "Promise", class_Promise_methods);
//    class_entry = zend_register_internal_class_ex(&ce, NULL);
//
//    zend_declare_class_constant_long(promise_class_entry,
//                                     "DEFAULT_FACTOR", sizeof("DEFAULT_FACTOR")-1, DEFAULT_SCALE_FACTOR);
//    zend_declare_property_long(promise_class_entry,
//                               "status", sizeof("status")-1, DEFAULT_SCALE_FACTOR, ZEND_ACC_PRIVATE);
//    zval property_service_default_value;
//    ZVAL_NULL(&property_service_default_value);
//    zend_string *property_service_name = zend_string_init("service", sizeof("service") - 1, 1);
//    zend_declare_property_ex(class_entry, property_service_name, &property_service_default_value, ZEND_ACC_PRIVATE, NULL);
//    zend_string_release(property_service_name);
//
//    zend_string *property___soap_fault_class_SoapFault = zend_string_init("SoapFault", sizeof("SoapFault")-1, 1);
//    zval property___soap_fault_default_value;
//    ZVAL_NULL(&property___soap_fault_default_value);
//    zend_string *property___soap_fault_name = zend_string_init("__soap_fault", sizeof("__soap_fault") - 1, 1);
//    zend_declare_typed_property(class_entry, property___soap_fault_name, &property___soap_fault_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property___soap_fault_class_SoapFault, 0, MAY_BE_NULL));
//    zend_string_release(property___soap_fault_name);
//
//    return class_entry;
//}
