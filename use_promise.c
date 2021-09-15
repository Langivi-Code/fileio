//
// Created by admin on 14.09.2021.
//
#include <php.h>

PHP_FUNCTION (retbool) {
    RETURN_BOOL(INI_BOOL("file_io.use_promise"));
}