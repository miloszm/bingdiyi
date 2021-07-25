#include "bing_config.hpp"

// "tcp://mainnet.libbitcoin.net:9091"
// "tcp://mainnet2.libbitcoin.net:9091"
const string BingConfig::libbitcoin_server_url {"tcp://testnet2.libbitcoin.net:19091"};
const uint8_t BingConfig::libbitcoin_connection_retries {3};
const uint16_t BingConfig::libbitcoin_connection_timeout_seconds {8};

//  ("localhost", "51002");
//  ("blockstream.info", "993");
//  ("testnet.qtornado.com", "51002");
//  ("testnet.electrumx.hodlwallet.com", "51002");
//  ("testnet.hsmiths.com", "53012");
const string BingConfig::electrum_server_host {"testnet.electrumx.hodlwallet.com"};
const string BingConfig::electrum_server_service {"51002"};
const string BingConfig::electrum_cert_file_path {"cert.crt"};

//  { "testnet.electrumx.hodlwallet.com", "51002", 0, ""},
//  { "testnet.hsmiths.com", "53012", 0, "" },
//  { "testnet.qtornado.com", "51002", 0, "" },
//  { "testnet1.bauerj.eu", "50002", 0, "" },
//  { "tn.not.fyi", "55002", 0, "" },
//  { "electrum.blockstream.info", "60002", 0, "" },
//  { "testnet.aranguren.org", "51002", 0, "" },
//  { "blockstream.info", "993", 0, "" }
