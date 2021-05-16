dnl config.m4 for extension fileio

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary.

dnl If your extension references something external, use 'with':

dnl PHP_ARG_WITH([fileio],
dnl   [for fileio support],
dnl   [AS_HELP_STRING([--with-fileio],
dnl     [Include fileio support])])

dnl Otherwise use 'enable':

PHP_ARG_ENABLE([fileio],
  [whether to enable fileio support],
  [AS_HELP_STRING([--enable-fileio],
    [Enable fileio support])],
  [yes])

if test "$PHP_FILEIO" != "no"; then
  dnl Write more examples of tests here...

  dnl Remove this code block if the library does not support pkg-config.
  dnl PKG_CHECK_MODULES([LIBFOO], [foo])
  dnl PHP_EVAL_INCLINE($LIBFOO_CFLAGS)
  dnl PHP_EVAL_LIBLINE($LIBFOO_LIBS, FILEIO_SHARED_LIBADD)

  dnl If you need to check for a particular library version using PKG_CHECK_MODULES,
  dnl you can use comparison operators. For example:
  dnl PKG_CHECK_MODULES([LIBFOO], [foo >= 1.2.3])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo < 3.4])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo = 1.2.3])

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-fileio -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/fileio.h"  # you most likely want to change this
  dnl if test -r $PHP_FILEIO/$SEARCH_FOR; then # path given as parameter
  dnl   FILEIO_DIR=$PHP_FILEIO
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for fileio files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       FILEIO_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$FILEIO_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the fileio distribution])
  dnl fi

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-fileio -> add include path
  dnl PHP_ADD_INCLUDE($FILEIO_DIR/include)

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-fileio -> check for lib and symbol presence
  dnl LIBNAME=FILEIO # you may want to change this
  dnl LIBSYMBOL=FILEIO # you most likely want to change this

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   AC_DEFINE(HAVE_FILEIO_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your fileio library.])
  dnl ], [
  dnl   $LIBFOO_LIBS
  dnl ])

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are not using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $FILEIO_DIR/$PHP_LIBDIR, FILEIO_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_FILEIO_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your fileio library.])
  dnl ],[
  dnl   -L$FILEIO_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(FILEIO_SHARED_LIBADD)

  dnl In case of no dependencies
  AC_DEFINE(HAVE_FILEIO, 1, [ Have fileio support ])

  PHP_NEW_EXTENSION(fileio, fileio.c, $ext_shared)
fi
