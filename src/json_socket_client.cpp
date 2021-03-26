#include "json_socket_client.hpp"
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

JsonSocketClient::JsonSocketClient(boost::asio::io_context &io_context,
                                   boost::asio::ssl::context &context,
                                   const tcp::resolver::results_type &endpoints)
    : socket_(io_context, context) {
  prepare_connection.lock();
  socket_.set_verify_mode(boost::asio::ssl::verify_none);
  socket_.set_verify_callback(
      std::bind(&JsonSocketClient::verify_certificate, this, _1, _2));

  connect(endpoints);
}

bool JsonSocketClient::verify_certificate(
    bool preverified, boost::asio::ssl::verify_context &ctx) {
  // The verify callback can be used to check whether the certificate that is
  // being presented is valid for the peer. For example, RFC 2818 describes
  // the steps involved in doing this for HTTPS. Consult the OpenSSL
  // documentation for more details. Note that the callback is called once
  // for each certificate in the certificate chain, starting from the root
  // certificate authority.

  // In this example we will simply print the certificate's subject name.
  char subject_name[256];
  X509 *cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
  X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
  std::cout << "Verifying: " << subject_name << "\n";

  return preverified;
}

void JsonSocketClient::connect(const tcp::resolver::results_type &endpoints) {
  boost::asio::async_connect(socket_.lowest_layer(), endpoints,
                             [this](const boost::system::error_code &error,
                                    const tcp::endpoint & /*endpoint*/) {
                               if (!error) {
                                 handshake();
                               } else {
                                 std::cout
                                     << "Connect failed: " << error.message()
                                     << "\n";
                               }
                             });
}

void JsonSocketClient::handshake() {
  socket_.async_handshake(boost::asio::ssl::stream_base::client,
                          [this](const boost::system::error_code &error) {
                            if (!error) {
                              std::cout << "unlocking prepare_connection"
                                        << "\n";
                              prepare_connection.unlock();
                            } else {
                              std::cout
                                  << "Handshake failed: " << error.message()
                                  << "\n";
                            }
                          });
}

void JsonSocketClient::send_request(json json_request) {
  enum { max_length = 1024 };
  char request_[max_length];
  std::string req0 = json_request.dump();
  std::string req = req0 + "\n";
  strcpy(request_, req.data());
  size_t request_length = std::strlen(req.data());

  boost::asio::write(socket_, boost::asio::buffer(request_, request_length));
}

json JsonSocketClient::receive_response() {
  std::ostringstream oss;
  for (;;) {
    boost::array<char, 512> buf;
    boost::system::error_code error;

    size_t len = socket_.read_some(boost::asio::buffer(buf), error);

    if (error == boost::asio::error::eof)
      break; // Connection closed cleanly by peer.
    else if (error)
      throw boost::system::system_error(error); // Some other error.

    oss.write(buf.data(), len);
    std::string candidate_response = oss.str();
    try {
      json parsed_response = json::parse(candidate_response);
      return parsed_response;
    } catch (json::parse_error &e) {
      // not yet parsable, keep reading
      continue;
    }
  }
  return json::parse("{}");
}
