#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <nlohmann/json.hpp>

using boost::asio::ip::tcp;
using std::placeholders::_1;
using std::placeholders::_2;
using json = nlohmann::json;
using namespace std;
#include "electrum_client.hpp"

void electrum_request_to_json(nlohmann::json &j, const ElectrumRequest &r) {
  j = nlohmann::json{{"jsonrpc", "2.0"},
                     {"method", r.method},
                     {"id", r.id},
                     {"params", r.params}};
}

ElectrumClient::ElectrumClient() : io_context(new boost::asio::io_context()), ctx(new boost::asio::ssl::context(boost::asio::ssl::context::sslv23)) {}

ElectrumClient::~ElectrumClient() {
    delete client;
    delete io_context;
    delete ctx;
}

void ElectrumClient::init(string hostname, string service,
                          string cert_file_path) {
  tcp::resolver resolver(*io_context);
  endpoints = resolver.resolve(hostname, service);

  ctx->load_verify_file(cert_file_path);

  client = new JsonSocketClient(*io_context, *ctx, endpoints);
  io_context->run();
  client->prepare_connection.lock();
}

/**
 * NOTE
 * To really use this client, it is needed to implement
 * some form of correlation check, currently request id
 * correlation is not done
 */
json ElectrumClient::send_request(json json_request) {
  client->send_request(json_request);
  return client->receive_response();
}
