//
// Created by admin on 14.09.2021.
//
#include <php.h>
#include <php_ini.h>

PHP_FUNCTION (use_promise) {
    RETURN_BOOL(INI_BOOL("file_io.use_promise"));
}