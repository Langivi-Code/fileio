AC_ENABLE_STATIC(yes)

PHP_ARG_WITH(uv, Whether to include "uv" support,
[ --with-uv[=DIR]        Include "uv" support], no, yes)

PHP_ARG_ENABLE(uv-debug, for uv debug support,
    [ --enable-uv-debug       Enable enable uv debug support], no, no)

PHP_ARG_ENABLE(dtrace, Whether to enable the "dtrace" debug,
    [ --enable-dtrace         Enable "dtrace" support], no, no)

AC_DEFUN([GLOB_DIR],[
  AC_ARG_VAR([$1][_DIR], [Sources dir])dnl
  rpath=$(realpath $2)
  AC_MSG_NOTICE([Searching in $rpath for files...])
source=""
for i in $2/functions/*/*.c ; do
  # file=$(echo $i | sed -e "s|$2/functions|\$FUNCTIONS|g")
  source="$source $i"
done
  $1[]_DIR=$source
])


if test $PHP_UV != "no"; then
GLOB_DIR([FILE_IO],[.])
 AC_MSG_NOTICE($FILE_IO_DIR)
    SOURCES=""
    FUNCTIONS="functions"
    TIMERS="$FUNCTIONS/timers"
    COMMON="$FUNCTIONS/common"
    FILES="$FUNCTIONS/files"

    AC_MSG_CHECKING(for libuv)
    PKG_CHECK_MODULES([LIBUV],[libuv >= 1.30],[
      AC_MSG_NOTICE($LIBUV_CFLAGS)
      AC_MSG_NOTICE($LIBUV_LIBS)
    ],[
      PHP_CHECK_LIBRARY(uv, uv_version,
      [
        PHP_ADD_LIBRARY_WITH_PATH(uv, $UV_DIR/$PHP_LIBDIR, UV_SHARED_LIBADD)
        AC_DEFINE(HAVE_UVLIB,1,[ ])
      ],[
         [[ -d deps ]] || mkdir deps
         cd deps && [[ -d libuv ]] || git clone https://github.com/libuv/libuv.git -b v1.42.0
         cd libuv && sh autogen.sh && ./configure && make -j $(nproc)  && make install
         cd ../..
         ./configure
            SEARCH_PATH="/usr/local /usr"
            SEARCH_FOR="/include/uv.h"
      if test -r $PHP_UV/$SEARCH_FOR; then # path given as parameter
         UV_DIR=$PHP_UV
         AC_MSG_RESULT(from option: found in $UV_DIR)
      else # search default path list
         for i in $SEARCH_PATH ; do
             if test -r $i/$SEARCH_FOR; then
               UV_DIR=$i
               AC_MSG_RESULT(from default path: found in $i)
             fi
         done
      fi
      PHP_ADD_INCLUDE($UV_DIR/include)
      ],[
        -L$UV_DIR/$PHP_LIBDIR -lm
      ])
    ])

    PHP_EVAL_LIBLINE($LIBUV_LIBS, UV_SHARED_LIBADD)
    PHP_EVAL_INCLINE($LIBUV_CFLAGS)
    SOURCES="fileio.c \
     $COMMON/fill_event_handle.c \
     $COMMON/callback.c \
     $TIMERS/fill_timer_handle_with_data.c \
     $TIMERS/set_timeout.c \
     $TIMERS/set_interval.c \
     $FILES/fs_handle_map.c \
     $FILES/fill_fs_handle_with_data.c \
     $FILES/file_get_contents_async.c \
     $FILES/file_put_contents_async.c \
     $FUNCTIONS/use_promise/use_promise.c \
     $FUNCTIONS/idle/idle.c"
    PHP_NEW_EXTENSION(fileio, $FILE_IO_DIR, $ext_shared)

    PHP_ADD_EXTENSION_DEP(uv, fiber, true )

    AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

      case $host in
          *linux*)
              CFLAGS="$CFLAGS -luv"
           ;;
           *darwin*)
              # CFLAGS="$CFLAGS -luv"
      esac
	PHP_SUBST([CFLAGS])
  PHP_SUBST(UV_SHARED_LIBADD)
  AC_MSG_WARN([hello $UV_SHARED_LIBADD])
fi
if test -z "$PHP_DEBUG"; then
    AC_ARG_ENABLE(debug,
    [  --enable-debug          compile with debugging symbols],[
        PHP_DEBUG=$enableval
    ],[    PHP_DEBUG=no
    ])
fi
CFLAGS="$CFLAGS -std=c11"
if test "$PHP_UV_DEBUG" != "no"; then
    CFLAGS="$CFLAGS -Wall -g -ggdb -O0 -DPHP_UV_DEBUG=1"
    AC_DEFINE(PśHP_UV_DEBUG, 1, [Enable uv debug support])
fi


# if test "$PHP_DTRACE" != "no"; then
#     dnl TODO: we should move this line to Makefile.frag or somewhere.
#     case $host in
#         *darwin*)
#              dtrace -h -s phpuv_dtrace.d
#              UV_SHARED_DEPENDENCIES=phpuv_dtrace.h
#              PHP_ADD_LIBRARY(dtrace, UV_SHARED_LIBADD)
#              AC_DEFINE(PHP_UV_DTRACE, 1, [Enable uv dtrace support])
#              PHP_SUBST(UV_SHARED_DEPENDENCIES)
#              PHP_ADD_MAKEFILE_FRAGMENT
#         ;;
#         *linux*)
#              echo "dtrace does not support this machine. currently OSX only"
#     esac
# fi