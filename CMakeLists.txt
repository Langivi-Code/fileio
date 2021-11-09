cmake_minimum_required(VERSION 3.21)

project(fileio
        VERSION 1.0.0
        LANGUAGES C)
set(CMAKE_C_STANDARD 23)
message(STATUS "Begin cmaking of PHP extension ...")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
            "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=gnu17")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -O0 -Wall -std=gnu11 -fvisibility=hidden")

set(PHP_VERSION 8.0.1)
get_filename_component(PHP_SRC_PATH "../../" ABSOLUTE)


if (EXISTS "${PHP_SRC_PATH}/main")
    message("We are in PHP folder already")
elseif (EXISTS "/usr/include/php/20210902/") # Linux configs
    set(D main;Zend;ext;sapi)
    set(CMAKE_INCLUDE_PATH "/usr/include/php/20210902/;${CMAKE_INCLUDE_PATH}")
    foreach (X IN LISTS D)
        message(STATUS "X=${X}")
        set(CMAKE_INCLUDE_PATH "/usr/include/php/20210902/${X};${CMAKE_INCLUDE_PATH}")
    endforeach ()
    set(PHP_SRC_PATH "/usr/include/php/20210902/")
elseif (EXISTS "/usr/local/Cellar/php@8.1/8.1.0_2/include/php/")  # MacOS configs
    set(D main;Zend;ext;sapi)
    set(CMAKE_INCLUDE_PATH "/usr/local/Cellar/php@8.1/8.1.0_2/include/php/;${CMAKE_INCLUDE_PATH}")
    foreach (X IN LISTS D)
        message(STATUS "X=${X}")
        set(CMAKE_INCLUDE_PATH "/usr/local/Cellar/php@8.1/8.1.0_2/include/php/${X};${CMAKE_INCLUDE_PATH}")
    endforeach ()
    set(PHP_SRC_PATH "/usr/local/Cellar/php@8.1/8.1.0_2/include/php/")
else ()
    if (NOT EXISTS "./php-src")
        execute_process(COMMAND git clone https://github.com/php/php-src.git -b PHP-8.1)
    endif ()
    get_filename_component(PHP_SRC_PATH "./cmake-build-debug/php-src/" ABSOLUTE)
    message(${PHP_SRC_PATH})

    set(D main;Zend;ext;sapi)
    set(CMAKE_INCLUDE_PATH "${PHP_SRC_PATH};${CMAKE_INCLUDE_PATH}")
    foreach (X IN LISTS D)
        message(STATUS "X=${X}")
        set(CMAKE_INCLUDE_PATH "${PHP_SRC_PATH}/${X};${CMAKE_INCLUDE_PATH}")
    endforeach ()
    message(${CMAKE_INCLUDE_PATH})
endif ()

include_directories(
        "${PHP_SRC_PATH}"
        "${PHP_SRC_PATH}/main"
        "${PHP_SRC_PATH}/Zend"
        "${PHP_SRC_PATH}/TSRM"
        "${PHP_SRC_PATH}/ext"
        "${PHP_SRC_PATH}/sapi"
        "/usr/local/Cellar/libuv/1.42.0/include/"
)
find_library(uvPath2 NAMES uv libuv)

FIND_PATH(LIBUV_INCLUDE_DIR NAMES uv.h)
FIND_PATH(PHP_INCLUDE_DIR NAMES php.h)
#message(${uvPath2} )
#message(${LIBUV_INCLUDE_DIR} )
message(${PHP_INCLUDE_DIR})

FILE(GLOB KernelHeaders ./kernel/*.h)
FILE(GLOB KernelSources ./kernel/*.c)
FILE(GLOB ExtensionHeaders ./functions/**/*.h)
FILE(GLOB ExtensionSources ./functions/**/*.c)

#FILE(GLOB uvHeaders /usr/local/Cellar/libuv/1.42.0/include/*.h)

set(SOURCE_FILES
        fileio.c
        php_fileio.h
        ${KernelHeaders}
        ${KernelSources}
        ${ExtensionHeaders}
        ${ExtensionSources}
        ${LIBUV_INCLUDE_DIR}
        constants.h)
message(${ExtensionSources})
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config.h")
    add_definitions(-DHAVE_CONFIG_H)
    set(SOURCE_FILES "${SOURCE_FILES};config.h")
endif ()

add_library(fileio SHARED
        ${SOURCE_FILES})

message(STATUS "End cmaking of PHP extension")