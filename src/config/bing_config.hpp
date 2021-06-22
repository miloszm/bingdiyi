#ifndef BING_CONFIG_HPP
#define BING_CONFIG_HPP

#include <string>
#include "src/common/bing_common.hpp"
#include <bitcoin/system.hpp>

using namespace std;


class BingConfig {
public:
    static const string libbitcoin_server_url;
    static const uint8_t libbitcoin_connection_retries;
    static const uint16_t libbitcoin_connection_timeout_seconds;
    static const string electrum_server_host;
    static const string electrum_server_service;
    static const string electrum_cert_file_path;
};


#endif