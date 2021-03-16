AC_INIT([bing], [0.0.2], [mm@example.com])

AM_INIT_AUTOMAKE

AC_PROG_CXX

AC_CONFIG_FILES([Makefile])

PKG_CHECK_MODULES([bitcoin_explorer], [libbitcoin-explorer >= 3.0.0])
AC_SUBST([bitcoin_explorer_PKG], ['libbitcoin-explorer >= 3.0.0'])
AC_SUBST([bitcoin_explorer_CPPFLAGS], [${bitcoin_explorer_CFLAGS}])
AC_MSG_NOTICE([bitcoin_explorer_CPPFLAGS : ${bitcoin_explorer_CPPFLAGS}])
AC_MSG_NOTICE([bitcoin_explorer_LIBS : ${bitcoin_explorer_LIBS}])

CXXFLAGS="-ggdb -I../include/ $CXXFLAGS -std=c++11 $bitcoin_explorer_CPPFLAGS $DEBUG -DWITH_ICU"

AC_OUTPUT
