#include "bing_config.hpp"

// "tcp://mainnet.libbitcoin.net:9091"
// "tcp://mainnet2.libbitcoin.net:9091"
const string BingConfig::libbitcoin_server_url {"tcp://testnet1.libbitcoin.net:19091"};

//  ("localhost", "51002");
//  ("blockstream.info", "993");
//  ("testnet.qtornado.com", "51002");
//  ("testnet.electrumx.hodlwallet.com", "51002");
const string BingConfig::electrum_server_host {"testnet.electrumx.hodlwallet.com"};
const string BingConfig::electrum_server_service {"51002"};
const string BingConfig::electrum_cert_file_path {"cert.crt"};
