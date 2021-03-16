#ifndef BING_CLIENT_HPP
#define BING_CLIENT_HPP


#include <bitcoin/system.hpp>
#include <bitcoin/client.hpp>

using namespace bc;

class BingClient {
public:
    void                            init();
    size_t                          fetchHeight();
    system::chain::points_value     fetchUtxo(const system::wallet::payment_address address, uint64_t satoshis, system::wallet::select_outputs::algorithm);
private:
    client::connection_settings connection;
    void doConnect(client::obelisk_client& client);
};

#endif