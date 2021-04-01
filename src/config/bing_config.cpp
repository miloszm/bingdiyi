#include "src/common/bing_common.hpp"
#include "bing_config.hpp"

// "tcp://mainnet.libbitcoin.net:9091"
// "tcp://mainnet2.libbitcoin.net:9091"
const string BingConfig::libbitcoin_server_url {"tcp://testnet1.libbitcoin.net:19091"};
const uint8_t BingConfig::libbitcoin_connection_retries {3};
const uint16_t BingConfig::libbitcoin_connection_timeout_seconds {8};

//  ("localhost", "51002");
//  ("blockstream.info", "993");
//  ("testnet.qtornado.com", "51002");
//  ("testnet.electrumx.hodlwallet.com", "51002");
const string BingConfig::electrum_server_host {"testnet.electrumx.hodlwallet.com"};
const string BingConfig::electrum_server_service {"51002"};
const string BingConfig::electrum_cert_file_path {"cert.crt"};
