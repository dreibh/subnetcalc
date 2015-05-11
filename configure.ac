AC_INIT([SubNetCalc], [2.4.2], [dreib@iem.uni-due.de])

AM_INIT_AUTOMAKE
AC_PREREQ(2.60)

AC_CONFIG_HEADERS([config.h:config.in])
AC_CONFIG_SRCDIR([src/subnetcalc.cc])

LT_INIT
AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_CXX


# ###### colorgcc ###########################################################
AC_ARG_ENABLE([colorgcc],
              [  --enable-colorgcc       enable colorgcc support ][default=no]],
              enable_colorgcc=$enableval, enable_colorgcc=no)
if test "${enable_colorgcc}" = "yes"; then
   AC_CHECK_PROG([COLORGCC], [colorgcc], [yes], [no])
   if test "$COLORGCC" == "yes"
   then
      AC_MSG_NOTICE([Compiling using colorgcc])
      CC=colorgcc
      CXX=colorgcc -lstdc++
   fi
fi


# ###### Operating system checks ############################################
case $host_os in
freebsd*)
    ;;
linux*)
    ;;
openbsd*)
    ;;
solaris*)
    LDFLAGS="$LDFLAGS -lsocket -lnsl -lrt -lresolv"
    ;;
darwin*)
    ;;
esac


# ###### Networking declarations ############################################
AC_CHECK_HEADERS(sys/types.h sys/socket.h netinet/in.h)

AC_MSG_CHECKING(for socklen_t)
AC_TRY_COMPILE([#ifdef HAVE_SYS_TYPES_H
                #include <sys/types.h>
                #endif
                #include <sys/socket.h>],
               [socklen_t x = 1; return 0;],
               [AC_MSG_RESULT(yes)],
               [AC_MSG_RESULT(int)
                AC_DEFINE(socklen_t, int, [Define a type for socklen_t.])])

AC_MSG_CHECKING(if stderr is a variable of type FILE* and not a macro)
AC_TRY_COMPILE([#include <stdio.h>],
               [FILE** stdlog = &stderr],
               [AC_DEFINE(HAVE_STDERR_FILEPTR, 1, "Define to 1 if stderr is a pointer.")
                AC_MSG_RESULT(yes)],
               [AC_MSG_RESULT(no)])

AC_MSG_CHECKING([whether 'struct sockaddr' has sa_len])
AC_TRY_COMPILE([#ifdef HAVE_SYS_TYPES_H
                #include <sys/types.h>
                #endif
                #include <sys/socket.h>],
               [static struct sockaddr sa; int i = sizeof(sa.sa_len);],
               [AC_MSG_RESULT(yes)
                AC_DEFINE(HAVE_SA_LEN,1,"Define this if you have sa_len in sockaddr struct.")],
                AC_MSG_RESULT(no))

AC_MSG_CHECKING([whether 'struct sockaddr_in' has sin_len])
AC_TRY_COMPILE([#ifdef HAVE_SYS_TYPES_H
                #include <sys/types.h>
                #endif
                #include <netinet/in.h>],
               [static struct sockaddr_in in; int i = sizeof(in.sin_len);],
               [AC_MSG_RESULT(yes)
                AC_DEFINE(HAVE_SIN_LEN,1,"Define this if your IPv4 has sin_len in sockaddr_in struct.")],
                AC_MSG_RESULT(no))

AC_MSG_CHECKING([whether 'struct sockaddr_in6' has sin6_len])
AC_TRY_COMPILE([#ifdef HAVE_SYS_TYPES_H
                #include <sys/types.h>
                #endif
                #include <netinet/in.h>],
               [static struct sockaddr_in6 in; int i = sizeof(in.sin6_len);],
               [AC_MSG_RESULT(yes)
                AC_DEFINE(HAVE_SIN6_LEN,1,"Define this if your IPv6 has sin6_len in sockaddr_in6 struct.")],
                AC_MSG_RESULT(no))


# ###### GeoIP ##############################################################
GEOIP_LIBS=''
AC_MSG_CHECKING(whether to use the GeoIP IP address mapping library if available)

AC_ARG_WITH(geoip,
  AC_HELP_STRING( [--with-geoip@<:@=DIR@:>@],
                  [use GeoIP (located in directory DIR, if supplied).   @<:@default=no, if present@:>@]),
                  [
                     if   test "x$withval" = "xno";  then
                        want_geoip=no
                     elif test "x$withval" = "xyes"; then
                        want_geoip=yes
                        CFLAGS="$CFLAGS -I/usr/local/include"
                        CXXFLAGS="$CXXFLAGS -I/usr/local/include"
                        LDFLAGS="$LDFLAGS -L/usr/local/lib"
                     elif test -d "$withval"; then
                        want_geoip=yes
                        CFLAGS="$CFLAGS -I${withval}/include"
                        CXXFLAGS="$CXXFLAGS -I${withval}/include"
                        LDFLAGS="$LDFLAGS -L${withval}/lib"
                     fi
                  ])
if test "x$want_geoip" = "xno"; then
   AC_MSG_RESULT(no)
else
   AC_MSG_RESULT(yes)
   AC_CHECK_LIB(GeoIP, GeoIP_open_type,
                [ GEOIP_LIBS=-lGeoIP
                  AC_DEFINE(HAVE_GEOIP, 1, [Define to use GeoIP library]) ],
                [ AC_ERROR([GeoIP not found!]) ],)
   AC_MSG_CHECKING(whether GeoIP supports IPv6)
   AC_TRY_COMPILE([#include <GeoIP.h>],
                  [GeoIPDBTypes test1 = GEOIP_COUNTRY_EDITION_V6;
                   int          test2 = GeoIP_id_by_addr(NULL, NULL);],
                  [AC_MSG_RESULT(yes)
                   AC_DEFINE(HAVE_GEOIP_IPV6, 1, "Define to 1 if GeoIP supports IPv6.")],
                  AC_MSG_RESULT(no))
fi
AC_SUBST(GEOIP_LIBS)


# ###### Makefiles ##########################################################
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
