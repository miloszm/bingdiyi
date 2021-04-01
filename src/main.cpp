#include <iostream>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client.hpp>
#include "src/config/bing_config.hpp"


using namespace bc;



// $ g++ -std=c++11 -o height HelloBlockChain.cpp $(pkg-config --cflags libbitcoin --libs libbitcoin libbitcoin-client)


/**
 * demonstrates programmatic call of a bx command
 */

int main() {

    client::connection_type connection = {};
    connection.retries = 3;
    connection.timeout_seconds = 8;
    connection.server = config::endpoint(BingConfig::libbitcoin_server_url);


    static const auto on_error = [](const code& ec)
    {
        std::cout << "Error Code: " << ec.message() << std::endl;
    };

    auto on_reply = [](size_t blockHeight)
    {
        std::cout << "Height: " << blockHeight << std::endl;
    };

    client::obelisk_client client(connection);

    if(!client.connect(connection))
    {
        std::cout << "Fail" << std::endl;
    } else {
        std::cout << "Connection Succeeded" << std::endl;
    }

    client.blockchain_fetch_last_height(on_error, on_reply);
    client.wait();

	std::cout << "Hello bing (testnet) 4\n";

}
