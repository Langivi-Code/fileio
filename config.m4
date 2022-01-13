PHP_ARG_WITH(uv, Whether to include "uv" support,
[ --with-uv[=DIR]        Include "uv" support])

PHP_ARG_ENABLE(uv-debug, for uv debug support,
    [ --enable-uv-debug       Enable enable uv debug support], no, no)

PHP_ARG_ENABLE(dtrace, Whether to enable the "dtrace" debug,
    [ --enable-dtrace         Enable "dtrace" support], no, no)

AX_CHECK_COMPILE_FLAG([-std=c11], [
  CFLAGS+=" -std=c11"
], [
  echo "C compiler cannot compile C11 code"
  exit -1
])

AC_DEFUN([GLOB_DIR],[
  get_abs_filename() {
  # $1 : relative filename
  filename=$1
  parentdir=$(dirname "${filename}")

  if [[ -d "${filename}" ]]; then
      echo "$(cd "${filename}" && pwd)"
  elif [[ -d "${parentdir}" ]]; then
    echo "$(cd "${parentdir}" && pwd)"
  fi
}
  AC_ARG_VAR([$1][_DIR], [Sources dir])dnl
  rpath=$(get_abs_filename $2)

  AC_MSG_NOTICE([Searching in $rpath for files...])
source=""
for i in $rpath/functions/*/*.c ; do
  file=$(echo $i | sed -e "s|$rpath/| |g")
  source="$source $file"
done
for i in $rpath/3rd/*/*.c ; do
  file=$(echo $i | sed -e "s|$rpath/| |g")
  source="$source $file"
done
  $1[]_DIR=$source
   AC_MSG_NOTICE($source)
])

if test "$PHP_UV_DEBUG" != "no"; then
    CFLAGS="$CFLAGS -Wall -g -ggdb -O0 -DPHP_UV_DEBUG=1"
    AC_DEFINE(PHP_UV_DEBUG, 1, [Enable uv debug support])
fi


if test "$PHP_DTRACE" != "no"; then
    dnl TODO: we should move this line to Makefile.frag or somewhere.
    case $host in
        *darwin*)
             dtrace -h -s phpuv_dtrace.d
             UV_SHARED_DEPENDENCIES=phpuv_dtrace.h
             PHP_ADD_LIBRARY(dtrace, UV_SHARED_LIBADD)
             AC_DEFINE(PHP_UV_DTRACE, 1, [Enable uv dtrace support])
             PHP_SUBST(UV_SHARED_DEPENDENCIES)
             PHP_ADD_MAKEFILE_FRAGMENT
        ;;
        *linux*)
             echo "dtrace does not support this machine. currently OSX only"
    esac
fi

if test $PHP_UV != "no"; then
    SOURCES=""
    # FUNCTIONS="functions"
    # TIMERS="$FUNCTIONS/timers"
    # COMMON="$FUNCTIONS/common"
    # FILES="$FUNCTIONS/files"
    GLOB_DIR([FILE_IO],[.])
    echo "$FILE_IO_DIR dir"
    # PHP_NEW_EXTENSION(fileio, fileio.c $COMMON/fill_event_handle.c $COMMON/callback.c $TIMERS/fill_timer_handle_with_data.c $TIMERS/set_timeout.c $TIMERS/set_interval.c $FILES/fs_handle_map.c $FILES/fill_fs_handle_with_data.c $FILES/file_get_contents_async.c $FILES/file_put_contents_async.c $FUNCTIONS/use_promise/use_promise.c $FUNCTIONS/idle/idle.c $SOURCES, $ext_shared)
    PHP_NEW_EXTENSION(fileio, fileio.c $FILE_IO_DIR,$ext_shared)
    PHP_ADD_EXTENSION_DEP(fileio, mysqli pgsql)

    AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

    AC_MSG_CHECKING(for libuv)

    if test $PHP_UV == "yes" && test -x "$PKG_CONFIG" && $PKG_CONFIG --exists libuv; then
      if $PKG_CONFIG libuv --atleast-version 1.0.0; then
        LIBUV_INCLINE=`$PKG_CONFIG libuv --cflags`
        LIBUV_LIBLINE=`$PKG_CONFIG libuv --libs`
        LIBUV_VERSION=`$PKG_CONFIG libuv --modversion`
        AC_MSG_RESULT(from pkgconfig: found version $LIBUV_VERSION)
        AC_DEFINE(HAVE_UVLIB,1,[ ])
      else
        AC_MSG_ERROR(system libuv must be upgraded to version >= 1.0.0)
      fi
      PHP_EVAL_LIBLINE($LIBUV_LIBLINE, UV_SHARED_LIBADD)
      PHP_EVAL_INCLINE($LIBUV_INCLINE)

    else
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
          #AC_MSG_ERROR([wrong uv library version or library not found])
      ],[
        -L$UV_DIR/$PHP_LIBDIR -lm
      ])

    fi
      case $host in
          *linux*)
              CFLAGS="$CFLAGS -luv -std=gnu11"
           ;;
           *darwin*)
              CFLAGS="$CFLAGS -luv -std=gnu11"
      esac
	PHP_SUBST([CFLAGS])
    PHP_SUBST(UV_SHARED_LIBADD)
fi
