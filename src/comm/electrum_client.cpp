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

ElectrumClient::ElectrumClient() {}

ElectrumClient::~ElectrumClient() { delete client; }

void ElectrumClient::init(string hostname, string service,
                          string cert_file_path) {
  boost::asio::io_context io_context;
  tcp::resolver resolver(io_context);
  auto endpoints = resolver.resolve(hostname, service);

  boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
  ctx.load_verify_file(cert_file_path);

  client = new JsonSocketClient(io_context, ctx, endpoints);
  io_context.run();
  client->prepare_connection.lock();
}

json ElectrumClient::send_request(json json_request) {
  client->send_request(json_request);
  return client->receive_response();
}