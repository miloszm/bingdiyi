#ifndef BING_CONFIG_HPP
#define BING_CONFIG_HPP

#include <string>

using namespace std;


class BingConfig {
public:
    static const string libbitcoin_server_url;
    static const string electrum_server_host;
    static const string electrum_server_service;
    static const string electrum_cert_file_path;
};


#endif