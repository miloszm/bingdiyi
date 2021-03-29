bin_PROGRAMS = crlocktx crunlocktx crlocktx2 crlocktx3 crtx eclient aconv


crtx_SOURCES = src/cmdtool/raw_transaction_producer.cpp

crlocktx_SOURCES = src/libbscript/redeem_script.cpp \
                   src/cmdtool/locked_tx_pusher.cpp

crlocktx2_SOURCES = src/locktx/online_lock_tx_creator.cpp \
                    src/libbfunds/funds_finder.cpp \
                    src/libbscript/redeem_script.cpp \
                    src/libbitcoinclient/libb_client.cpp \
                    src/cmdtool/locked_tx_autopusher.cpp

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

aconv_SOURCES = src/cmdtool/address_spkh.cpp


crtx_LDADD= @bitcoin_client_LIBS@

crlocktx_LDADD= @bitcoin_client_LIBS@

crlocktx2_LDADD= @bitcoin_client_LIBS@

crlocktx3_LDADD= @bitcoin_client_LIBS@

crunlocktx_LDADD= @bitcoin_client_LIBS@

eclient_LDADD= @openssl_libcrypto_LIBS@ @openssl_libssl_LIBS@

aconv_LDADD= @libbinglib_LIBS@


#check_PROGRAMS = test1
#test1_SOURCES = \
#    test/utility/address_converter.cpp \
#    src/utility/address_converter.cpp
#test1_CPPFLAGS = -I$(top_srcdir)/src
#TESTS = $(check_PROGRAMS)

