AC_INIT([bing], [0.0.2], [mm@example.com])

AM_INIT_AUTOMAKE

AC_PROG_CXX

AC_CONFIG_FILES([Makefile])

PKG_CHECK_MODULES([bitcoin_client], [libbitcoin-client >= 3.0.0])
AC_SUBST([bitcoin_client_PKG], ['libbitcoin-client >= 3.0.0'])
AC_SUBST([bitcoin_client_CPPFLAGS], [${bitcoin_client_CFLAGS}])
AC_MSG_NOTICE([bitcoin_client_CPPFLAGS : ${bitcoin_client_CPPFLAGS}])
AC_MSG_NOTICE([bitcoin_client_LIBS : ${bitcoin_client_LIBS}])

CXXFLAGS="-ggdb -I../include/ $CXXFLAGS -std=c++11 $bitcoin_client_CPPFLAGS $DEBUG -DWITH_ICU"

AC_OUTPUT
