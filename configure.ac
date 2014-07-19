AC_INIT([APL plot native function], [1.0], [moller@mollerware.com],[aplplot])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_MAINTAINER_MODE([enable])

#AC_PROG_CC
AC_PROG_CXX
AM_PROG_AR
AM_PROG_LIBTOOL

# this has to point into the GNU apl source tree
APL_SOURCES=-I../../apl-1.3/src/native
AC_SUBST(APL_SOURCES)

AC_CHECK_PROG([IMG_DISPLAY], [display], [yes], [])
m4_ifblank([$IMG_DISPLAY],
  [echo "$IMG_DISPLAY";
  echo ">>>> You need to install the ImageMagick package. <<<<";
  echo ""; exit])

# AC_CHECK_HEADERS tries to compile the header but uses C instead of C++
# maybe an autoconf expert knows how to make it work right
#AC_CHECK_HEADERS([boost/spirit/include/qi.hpp], ,
#  [echo "";
#  echo ">>>> You need to install the boost packages. <<<<";
#  echo ""; exit], AC_INCLUDES_DEFAULT)

# should use AC_CHECK_HEADERS, but see above
AC_CHECK_FILE([/usr/include/boost/spirit/include/qi.hpp], ,
  [echo "";
  echo ">>>> You need to install the boost packages. <<<<";
  echo ""; exit])

PKG_CHECK_MODULES([PLPLOT], [plplotd >= 5.9.9], ,
  [echo "";
  echo ">>>> You need to install the plplot and plplot-devel packages. <<<<";
  echo ""; exit])
AC_SUBST(PLPLOT_CFLAGS)
AC_SUBST(PLPLOT_LIBS)

AC_CONFIG_FILES([
  Makefile
  src/Makefile
])
AC_OUTPUT
