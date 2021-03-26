bin_PROGRAMS = crlocktx crunlocktx crlocktx2 crlocktx3 crtx eclient


crtx_SOURCES = src/raw_transaction_producer.cpp

crlocktx_SOURCES = src/redeem_script.cpp src/locked_tx_pusher.cpp

crlocktx2_SOURCES = src/autopusher.cpp src/funds_finder.cpp src/redeem_script.cpp src/bing_client.cpp src/locked_tx_autopusher2.cpp

crlocktx3_SOURCES = src/autopusher.cpp src/purse_accessor.cpp src/funds_finder.cpp src/redeem_script.cpp src/bing_client.cpp src/locked_tx_autopusher_seed.cpp

crunlocktx_SOURCES = src/redeem_script.cpp src/locked_tx_spender.cpp

eclient_SOURCES = src/comm/json_socket_client.cpp src/comm/electrum_client.cpp src/comm/eclient.cpp


crtx_LDADD= @bitcoin_client_LIBS@

crlocktx_LDADD= @bitcoin_client_LIBS@

crlocktx2_LDADD= @bitcoin_client_LIBS@

crlocktx3_LDADD= @bitcoin_client_LIBS@

crunlocktx_LDADD= @bitcoin_client_LIBS@

eclient_LDADD= @openssl_libcrypto_LIBS@ @openssl_libssl_LIBS@
