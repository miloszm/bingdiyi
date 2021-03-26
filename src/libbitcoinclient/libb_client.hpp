#ifndef BING_CLIENT_HPP
#define BING_CLIENT_HPP

#include "src/common/bing_common.hpp"
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client.hpp>

using namespace bc;

class LibbClient {
public:
  void init();
  size_t fetch_height();
  chain::points_value fetch_utxo(const wallet::payment_address address,
                                uint64_t satoshis,
                                wallet::select_outputs::algorithm);

private:
  client::connection_type connection;
  void do_connect(client::obelisk_client &client);
};

#endif