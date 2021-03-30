#ifndef ELECTRUM_CLIENT_HPP
#define ELECTRUM_CLIENT_HPP

#include "json_socket_client.hpp"
#include <string>

using namespace std;

struct ElectrumRequest {
    string method;
    int id;
    string params;
};

void electrum_request_to_json(nlohmann::json& j, const ElectrumRequest& r) {
    j = nlohmann::json{{"jsonrpc", "2.0"}, {"method", r.method}, {"id", r.id}, {"params", r.params}};
}

class ElectrumClient {
public:
    ElectrumClient();
    virtual ~ElectrumClient();
    void init(string hostname, string service, string certificationFilePath);
    nlohmann::json send_request(nlohmann::json json_request);
private:
    JsonSocketClient* client;
};

#endif
