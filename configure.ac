AC_INIT([bing], [0.0.2], [mm@example.com])

AM_INIT_AUTOMAKE

AC_PROG_CXX

AC_CONFIG_FILES([Makefile])

PKG_CHECK_MODULES([bitcoin_client], [libbitcoin-client >= 3.0.0])
AC_SUBST([bitcoin_client_PKG], ['libbitcoin-client >= 3.0.0'])
AC_SUBST([bitcoin_client_CPPFLAGS], [${bitcoin_client_CFLAGS}])
AC_MSG_NOTICE([bitcoin_client_CPPFLAGS : ${bitcoin_client_CPPFLAGS}])
AC_MSG_NOTICE([bitcoin_client_LIBS : ${bitcoin_client_LIBS}])

PKG_CHECK_MODULES([libjsonrpccpp_client], [libjsonrpccpp-client >= 1.3.0])
AC_SUBST([libjsonrpccpp_client_PKG], ['libjsonrpccpp-client >= 1.3.0'])
AC_SUBST([libjsonrpccpp_client_CPPFLAGS], [${libjsonrpccpp_client_CFLAGS}])
AC_MSG_NOTICE([libjsonrpccpp_client_CPPFLAGS : ${libjsonrpccpp_client_CPPFLAGS}])
AC_MSG_NOTICE([libjsonrpccpp_client_LIBS : ${libjsonrpccpp_client_LIBS}])

PKG_CHECK_MODULES([libjsonrpccpp_common], [libjsonrpccpp-common >= 1.3.0])
AC_SUBST([libjsonrpccpp_common_PKG], ['libjsonrpccpp-common >= 1.3.0'])
AC_SUBST([libjsonrpccpp_common_CPPFLAGS], [${libjsonrpccpp_common_CFLAGS}])
AC_MSG_NOTICE([libjsonrpccpp_common_CPPFLAGS : ${libjsonrpccpp_common_CPPFLAGS}])
AC_MSG_NOTICE([libjsonrpccpp_common_LIBS : ${libjsonrpccpp_common_LIBS}])

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


CXXFLAGS="-ggdb -I../include/ $CXXFLAGS -std=c++14 $bitcoin_client_CPPFLAGS $openssl_libcrypto_CPPFLAGS $openssl_libssl_CPPFLAGS $nlohmann_json_CPPFLAGS $DEBUG"

AC_OUTPUT
