#ifndef ELECTRUM_CLIENT_HPP
#define ELECTRUM_CLIENT_HPP

#include "json_socket_client.hpp"
#include <string>


class ElectrumClient {
public:
    ElectrumClient();
    virtual ~ElectrumClient();
    void init(std::string hostname, std::string service, std::string certificationFilePath);
    nlohmann::json send_request(nlohmann::json json_request);
private:
    JsonSocketClient* client;
};

#endif
