cmake_minimum_required(VERSION 3.20)

project(fileio
        VERSION 1.0.0
        LANGUAGES C)

message(STATUS "Begin cmaking of PHP extension ...")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
            "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
#if (NOT CMAKE_C_STANDARD)
#    set(CMAKE_C_STANDARD 11)
#endif ()


set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=gnu11")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -O0 -Wall -std=gnu11 -fvisibility=hidden")

set(PHP_VERSION 8.0.1)
get_filename_component(PHP_SRC_PATH "../../" ABSOLUTE)

include_directories(
        "${PHP_SRC_PATH}"
        "${PHP_SRC_PATH}/main"
        "${PHP_SRC_PATH}/Zend"
        "${PHP_SRC_PATH}/TSRM"
        "${PHP_SRC_PATH}/ext"
        "${PHP_SRC_PATH}/sapi"
        "/usr/local/Cellar/libuv/1.42.0/include/"
        "/usr/local/Cellar/gcc/11.2.0/include/"

)
find_file(uvPath uv.h /)
find_library(uvPath2 NAMES uv libuv)
FIND_PATH(LIBUV_INCLUDE_DIR NAMES uv.h)
message(${uvPath2} )
message(${LIBUV_INCLUDE_DIR} )
FILE(GLOB KernelHeaders ./kernel/*.h)
FILE(GLOB KernelSources ./kernel/*.c)
#FILE(GLOB uvHeaders /usr/local/Cellar/libuv/1.42.0/include/*.h)

set(SOURCE_FILES
        fileio.c
        use_promise.c
        set_timeout.c
        php_fileio.h
        ${KernelHeaders}
        ${KernelSources}
        ${LIBUV_INCLUDE_DIR}
        /usr/local/Cellar/gcc/11.2.0/include/
        "${PHP_SRC_PATH}/Zend"
        set_timeout_interface.h file_get_contents_async.c callback_interface.h callback.c)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config.h")
    add_definitions(-DHAVE_CONFIG_H)
    set(SOURCE_FILES "${SOURCE_FILES};config.h" file_get_contents_async.c)
endif()

add_library(fileio SHARED
        ${SOURCE_FILES})

message(STATUS "End cmaking of PHP extension")