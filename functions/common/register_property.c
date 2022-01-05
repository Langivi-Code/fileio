//
// Created by admin on 12.12.2021.
//

#include "register_property.h"
#include <php.h>
#include <zend_API.h>

void register_property(zend_class_entry *ce, char *name, size_t len, zval *def_val, int access_type, int type) {
    zend_string * property = zend_string_init(name, len, 1);
    zend_declare_typed_property(ce, property, def_val, access_type, NULL, (zend_type) ZEND_TYPE_INIT_MASK(type));
    zend_string_release(property);
}

void
register_class_property(zend_class_entry *ce, char *name, size_t len, zval *def_val, int access_type, char *class_name,
                        size_t class_len) {
    zend_string * property = zend_string_init(name, len, 1);
    zend_string * property_class = zend_string_init(class_name, class_len, 1);
    zend_declare_typed_property(ce, property, def_val, access_type, NULL,
                                (zend_type) ZEND_TYPE_INIT_CLASS(property_class, 0, MAY_BE_NULL));
    zend_string_release(property);
}