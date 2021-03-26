#ifndef JSON_SOCKET_CLIENT_HPP
#define JSON_SOCKET_CLIENT_HPP

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <nlohmann/json.hpp>

class JsonSocketClient {
public:
  JsonSocketClient(
      boost::asio::io_context &io_context, boost::asio::ssl::context &context,
      const boost::asio::ip::tcp::resolver::results_type &endpoints);
  void send_request(nlohmann::json json_request);
  nlohmann::json receive_response();
  std::mutex prepare_connection;

private:
  bool verify_certificate(bool preverified,
                          boost::asio::ssl::verify_context &ctx);
  void connect(const boost::asio::ip::tcp::resolver::results_type &endpoints);
  void handshake();

  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
};

#endif
