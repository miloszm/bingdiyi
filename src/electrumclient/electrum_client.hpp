#ifndef ELECTRUM_CLIENT_HPP
#define ELECTRUM_CLIENT_HPP

#include "json_socket_client.hpp"
#include <string>

using namespace std;
using boost::asio::ip::tcp;

struct ElectrumRequest {
    string method;
    int id;
    vector<string> params;
};

void electrum_request_to_json(nlohmann::json& j, const ElectrumRequest& r);


class ElectrumClient {
public:
    ElectrumClient();
    virtual ~ElectrumClient();
    void init(string hostname, string service, string certificationFilePath);
    nlohmann::json send_request(nlohmann::json json_request);
private:
    JsonSocketClient* client;
    boost::asio::io_context* io_context;
    boost::asio::ssl::context* ctx;
    tcp::resolver::results_type endpoints;
};

#endif
