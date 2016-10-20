 PHP_ARG_WITH(varnish, for varnish support,
 [  --with-varnish             Include varnish support])

if test "$PHP_VARNISH" != "no"; then

  AC_CHECK_HEADER(fcntl.h)
  AC_CHECK_HEADER(sys/types.h)
  AC_CHECK_HEADER(sys/socket.h)
  AC_CHECK_HEADER(netinet/in.h)
  AC_CHECK_HEADER(arpa/inet.h)
  AC_CHECK_HEADER(netdb.h)

  AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
  if test -x "$PKG_CONFIG" && $PKG_CONFIG varnishapi --exists ; then
    dnl # rely on pkg-config available information
    AC_MSG_CHECKING(varnish version)
    if $PKG_CONFIG varnishapi --atleast-version=3 ; then
      VARNISH_INCLUDE=`$PKG_CONFIG varnishapi --cflags`
      VARNISH_LIBRARY=`$PKG_CONFIG varnishapi --libs`
      VARNISH_VERSION=`$PKG_CONFIG varnishapi --modversion`
      AC_MSG_RESULT($VARNISH_VERSION)

      PHP_EVAL_INCLINE($VARNISH_INCLUDE)
      PHP_EVAL_LIBLINE($VARNISH_LIBRARY, VARNISH_SHARED_LIBADD)
      if $PKG_CONFIG varnishapi --atleast-version=5 ; then
        AC_DEFINE(HAVE_VARNISHAPILIB,50,[ ])
      elif $PKG_CONFIG varnishapi --atleast-version=4.1 ; then
        AC_DEFINE(HAVE_VARNISHAPILIB,41,[ ])
      elif $PKG_CONFIG varnishapi --atleast-version=4 ; then
        AC_DEFINE(HAVE_VARNISHAPILIB,40,[ ])
      else
        AC_DEFINE(HAVE_VARNISHAPILIB,30,[ ])
      fi
      AC_TYPE_UINTPTR_T
      AC_TYPE_UINT64_T
    else
      AC_MSG_RESULT(version too old)
      AC_MSG_ERROR(Please reinstall varnish version >= 3.0)
    fi
  else
    dnl # --with-varnish -> check with-path
    SEARCH_PATH="$PHP_VARNISH /usr/local /usr"
    SEARCH_FOR="varnishapi.h vcli.h"
    AC_MSG_CHECKING([for varnish files in default path])
    for i in $SEARCH_PATH ; do
      for j in $SEARCH_FOR ; do
        if test -r $i/include/varnish/$j; then
          VARNISH_INCDIR=$i/include/varnish
          VARNISH_LIBDIR=$i/$PHP_LIBDIR
          break
        elif test -r $i/include/$j; then
          VARNISH_INCDIR=$i/include
          VARNISH_LIBDIR=$i/$PHP_LIBDIR
          break
        fi
      done
    done

    if test -z "$VARNISH_INCDIR"; then
      AC_MSG_RESULT([not found])
      AC_MSG_ERROR([Please reinstall the varnish distribution])
    else
      AC_MSG_RESULT(headers found in $VARNISH_INCDIR)
    fi

    PHP_ADD_INCLUDE($VARNISH_INCDIR)
	AC_CHECK_HEADER([$VARNISH_INCDIR/vcli.h], [], AC_MSG_ERROR('vcli.h' header not found))

    LIBNAME=varnishapi
    LIBSYMBOL=VSM_New

    PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
    [
      PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $VARNISH_LIBDIR, VARNISH_SHARED_LIBADD)

      if test -f $VARNISH_INCDIR/varnishapi.h; then
        dnl this is 3.x or earlier
        AC_DEFINE(HAVE_VARNISHAPILIB,30,[ ])
        AC_CHECK_HEADER([$VARNISH_INCDIR/varnishapi.h], [], AC_MSG_ERROR('varnishapi.h' header not found))
        AC_CHECK_HEADER([$VARNISH_INCDIR/vsl.h], [], AC_MSG_ERROR('vsl.h' header not found))
      elif test -f $VARNISH_INCDIR/vapi/vsm.h; then
        dnl this is approx at least 4.1.x, for later will probably have to check for some specific stuff
        AC_DEFINE(HAVE_VARNISHAPILIB,41,[ ])
      else
        dnl this is approx at least 4.x, for later will probably have to check for some specific stuff
        AC_DEFINE(HAVE_VARNISHAPILIB,40,[ ])
        dnl there's an issue with two lines below, it might be because STLM macro isn't defined,
        dnl so the compiler decides them to be unusable
        dnl AC_CHECK_HEADER([$VARNISH_INCDIR/tbl/vsl_tags.h], [], AC_MSG_ERROR('tbl/vsl_tags.h' header not found))
        dnl AC_CHECK_HEADER([$VARNISH_INCDIR/vapi/vsl.h], [], AC_MSG_ERROR('vapi/vsl.h' header not found))
      fi
      AC_TYPE_UINTPTR_T
      AC_TYPE_UINT64_T
    ],[
      AC_MSG_ERROR([wrong varnishapi lib version or lib not found])
    ],[
      -L$VARNISH_LIBDIR -lm
    ])
  fi
  PHP_SUBST(VARNISH_SHARED_LIBADD)

  PHP_NEW_EXTENSION(varnish, varnish.c adm.c varnish_lib.c exception.c stat.c log.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(varnish, hash, true)

fi
