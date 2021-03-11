#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client.hpp>

using namespace bc;

class BingClient {
public:
    void init();
    size_t fetchHeight();
private:
    client::connection_type connection;
    void doConnect(client::obelisk_client& client);
};