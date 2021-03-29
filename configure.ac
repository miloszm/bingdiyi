AC_INIT([bing], [0.0.2], [mm@example.com])

AM_INIT_AUTOMAKE([subdir-objects])

AC_PROG_CXX

AC_CONFIG_FILES([Makefile])

PKG_CHECK_MODULES([bitcoin_client], [libbitcoin-client >= 3.0.0])
AC_SUBST([bitcoin_client_PKG], ['libbitcoin-client >= 3.0.0'])
AC_SUBST([bitcoin_client_CPPFLAGS], [${bitcoin_client_CFLAGS}])
AC_MSG_NOTICE([bitcoin_client_CPPFLAGS : ${bitcoin_client_CPPFLAGS}])
AC_MSG_NOTICE([bitcoin_client_LIBS : ${bitcoin_client_LIBS}])

PKG_CHECK_MODULES([openssl_libssl], [libssl])
AC_SUBST([openssl_libssl_PKG], ['OpenSSL-libssl'])
AC_SUBST([openssl_libssl_CPPFLAGS], [${openssl_libssl_CFLAGS}])
AC_MSG_NOTICE([openssl_libssl_CPPFLAGS : ${openssl_libssl_CPPFLAGS}])
AC_MSG_NOTICE([openssl_libssl_LIBS : ${openssl_libssl_LIBS}])

PKG_CHECK_MODULES([openssl_libcrypto], [libcrypto])
AC_SUBST([openssl_libcrypto_PKG], ['OpenSSL-libcrypto'])
AC_SUBST([openssl_libcrypto_CPPFLAGS], [${openssl_libcrypto_CFLAGS}])
AC_MSG_NOTICE([openssl_libcrypto_CPPFLAGS : ${openssl_libcrypto_CPPFLAGS}])
AC_MSG_NOTICE([openssl_libcrypto_LIBS : ${openssl_libcrypto_LIBS}])

PKG_CHECK_MODULES([nlohmann_json], [nlohmann_json])
AC_SUBST([nlohmann_json_PKG], ['nlohmann_json'])
AC_SUBST([nlohmann_json_CPPFLAGS], [${nlohmann_json_CFLAGS}])
AC_MSG_NOTICE([nlohmann_json_CPPFLAGS : ${nlohmann_json_CPPFLAGS}])
AC_MSG_NOTICE([nlohmann_json_LIBS : ${nlohmann_json_LIBS}])

PKG_CHECK_MODULES([libbinglib], [libbinglib])
AC_SUBST([libbinglib_PKG], ['libbinglib'])
AC_SUBST([libbinglib_CPPFLAGS], [${libbinglib_CFLAGS}])
AC_MSG_NOTICE([libbinglib_CPPFLAGS : ${libbinglib_CPPFLAGS}])
AC_MSG_NOTICE([libbinglib_LIBS : ${libbinglib_LIBS}])


CXXFLAGS="-ggdb -I../include/ $CXXFLAGS -std=c++14 $libbinglib_CPPFLAGS $bitcoin_client_CPPFLAGS $openssl_libcrypto_CPPFLAGS $openssl_libssl_CPPFLAGS $nlohmann_json_CPPFLAGS $DEBUG"

AC_OUTPUT
