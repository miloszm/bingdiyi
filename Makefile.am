bin_PROGRAMS = crlocktx crunlocktx crlocktx2 crlocktx3 crtx crtx2 eclient aconv main

main_SOURCES = src/config/bing_config.cpp \
               src/main.cpp

crtx_SOURCES = src/cmdtool/p2pkh_tx_pusher.cpp

crtx2_SOURCES = src/config/bing_config.cpp \
                src/cmdtool/p2pkh_tx_autopusher.cpp

crlocktx_SOURCES = src/cmdtool/locked_tx_pusher.cpp

crlocktx2_SOURCES = src/config/bing_config.cpp \
                    src/cmdtool/locked_tx_autopusher.cpp

crlocktx3_SOURCES = src/config/bing_config.cpp \
                    src/cmdtool/locked_tx_autopusher_seed.cpp

crunlocktx_SOURCES = src/cmdtool/locked_tx_spender.cpp

eclient_SOURCES = src/config/bing_config.cpp \
                  src/electrumclient/eclient.cpp

aconv_SOURCES = src/cmdtool/address_spkh.cpp


main_LDADD= @bitcoin_client_LIBS@

crtx_LDADD= @bitcoin_client_LIBS@

crtx2_LDADD= @bitcoin_client_LIBS@ @libbinglib_LIBS@

crlocktx_LDADD= @bitcoin_client_LIBS@ @libbinglib_LIBS@

crlocktx2_LDADD= @bitcoin_client_LIBS@ @libbinglib_LIBS@

crlocktx3_LDADD= @openssl_libcrypto_LIBS@ @openssl_libssl_LIBS@ @bitcoin_client_LIBS@ @libbinglib_LIBS@

crunlocktx_LDADD= @bitcoin_client_LIBS@ @libbinglib_LIBS@

eclient_LDADD= @openssl_libcrypto_LIBS@ @openssl_libssl_LIBS@ @libbinglib_LIBS@

aconv_LDADD= @libbinglib_LIBS@



