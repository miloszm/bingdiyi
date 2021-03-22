bin_PROGRAMS = crlocktx crunlocktx crlocktx2 crlocktx3

crlocktx_SOURCES = src/redeem_script.cpp src/locked_tx_pusher.cpp

crlocktx2_SOURCES = src/funds_finder.cpp src/redeem_script.cpp src/bing_client.cpp src/locked_tx_autopusher2.cpp

crlocktx3_SOURCES = src/purse_accessor.cpp src/funds_finder.cpp src/redeem_script.cpp src/bing_client.cpp src/locked_tx_autopusher_seed.cpp

crunlocktx_SOURCES = src/redeem_script.cpp src/locked_tx_spender.cpp

crlocktx_LDADD= @bitcoin_client_LIBS@

crlocktx2_LDADD= @bitcoin_client_LIBS@

crlocktx3_LDADD= @bitcoin_client_LIBS@

crunlocktx_LDADD= @bitcoin_client_LIBS@
