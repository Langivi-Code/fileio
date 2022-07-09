//
// Created by admin on 09.07.2022.
//
#include <php.h>
#include "find_class.h"

zend_class_entry *find_class_by_name(char *name) {
    zend_string * pg_z_name = zend_string_init(name, strlen(name), 0);
    zend_string * lcname = zend_string_tolower(pg_z_name);
    zend_class_entry * class_entry = zend_hash_find_ptr(EG(class_table), lcname);
    zend_string_release_ex(lcname, 0);
    zend_string_release_ex(pg_z_name, 0);
    return class_entry;
}

