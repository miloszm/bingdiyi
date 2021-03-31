#include "libb_client.hpp"

void LibbClient::init() {
  //connection.server = config::endpoint("tcp://mainnet.libbitcoin.net:9091");
  //connection.server = config::endpoint("tcp://mainnet2.libbitcoin.net:9091");
  connection.server = config::endpoint("tcp://testnet1.libbitcoin.net:19091");
  do_connect(client);
}

void LibbClient::do_connect(client::obelisk_client &client) {
  if (!client.connect(connection)) {
    std::cout << "Connection Failed" << std::endl;
  }
}

size_t LibbClient::fetch_height() {
  size_t height;

  const auto on_error = [](const code &ec) {
    std::cout << "Error Code: " << ec.message() << std::endl;
  };

  auto on_reply = [&height](size_t blockHeight) { height = blockHeight; };

  client.blockchain_fetch_last_height(on_error, on_reply);
  client.wait();
  return height;
}

chain::points_value
LibbClient::fetch_utxo(const wallet::payment_address address, uint64_t satoshis,
                       wallet::select_outputs::algorithm algorithm) {
  chain::points_value points_value;

  const auto on_error = [](const code &ec) {
    std::cout << "Error Code: " << ec.message() << std::endl;
  };

  auto on_reply = [&points_value](const chain::points_value &pv) {
    points_value = pv;
  };

  client.blockchain_fetch_unspent_outputs(on_error, on_reply, address, satoshis,
                                          algorithm);
  client.wait();

  return points_value;
}
