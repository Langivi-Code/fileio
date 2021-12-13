//
// Created by admin on 12.12.2021.
//

#ifndef FILEIO_REGISTER_PROPERTY_H
#define FILEIO_REGISTER_PROPERTY_H
#include <php.h>
 void register_property(zend_class_entry *ce, char *name, size_t len, zval *def_val, int access_type, int type);
 void
register_class_property(zend_class_entry *ce, char *name, size_t len, zval *def_val, int access_type, char *class_name,
                        size_t class_len);
#endif //FILEIO_REGISTER_PROPERTY_H
