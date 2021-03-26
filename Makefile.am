bin_PROGRAMS = crlocktx crunlocktx crlocktx2 crlocktx3 crtx eclient


crtx_SOURCES = src/cmdtool/raw_transaction_producer.cpp

crlocktx_SOURCES = src/libbscript/redeem_script.cpp \
                   src/cmdtool/locked_tx_pusher.cpp

crlocktx2_SOURCES = src/locktx/online_lock_tx_creator.cpp \
                    src/libbfunds/funds_finder.cpp \
                    src/libbscript/redeem_script.cpp \
                    src/libbitcoinclient/libb_client.cpp \
                    src/cmdtool/locked_tx_autopusher2.cpp

crlocktx3_SOURCES = src/locktx/online_lock_tx_creator.cpp \
                    src/libbfunds/purse_accessor.cpp \
                    src/libbfunds/funds_finder.cpp \
                    src/libbscript/redeem_script.cpp \
                    src/libbitcoinclient/libb_client.cpp \
                    src/cmdtool/locked_tx_autopusher_seed.cpp

crunlocktx_SOURCES = src/libbscript/redeem_script.cpp \
                     src/cmdtool/locked_tx_spender.cpp

eclient_SOURCES = src/electrumclient/json_socket_client.cpp \
                  src/electrumclient/electrum_client.cpp \
                  src/electrumclient/eclient.cpp


crtx_LDADD= @bitcoin_client_LIBS@

crlocktx_LDADD= @bitcoin_client_LIBS@

crlocktx2_LDADD= @bitcoin_client_LIBS@

crlocktx3_LDADD= @bitcoin_client_LIBS@

crunlocktx_LDADD= @bitcoin_client_LIBS@

eclient_LDADD= @openssl_libcrypto_LIBS@ @openssl_libssl_LIBS@
