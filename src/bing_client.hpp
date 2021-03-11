#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client.hpp>

using namespace bc;

class BingClient {
public:
    void                    init();
    size_t                  fetchHeight();
    chain::points_value     fetchUtxo(const wallet::payment_address address, uint64_t satoshis, wallet::select_outputs::algorithm);
private:
    client::connection_type connection;
    void doConnect(client::obelisk_client& client);
};