#include <iostream>
#include <bitcoin/bitcoin.hpp>
//#include <bitcoin/client/obelisk_client.hpp>
#include <bitcoin/client.hpp>


using namespace bc;


int main() {

    client::connection_type connection = {};
    connection.retries = 3;
    connection.timeout_seconds = 8;
    connection.server = config::endpoint("tcp://mainnet.libbitcoin.net:9091");


    static constexpr uint8_t num_retries = 0;
    static constexpr uint16_t timeout_seconds = 8;

    //static auto address = std::string("tcp://libbitcoin1.openbazaar.org:9091");
//    static auto address = std::string("tcp://mainnet.libbitcoin.net:9091");

    static const auto on_error = [](const code& ec)
    {
        std::cout << "Error Code: " << ec.message() << std::endl;
    };

    auto on_reply = [](size_t blockHeight)
    {
        std::cout << "Height: " << blockHeight << std::endl;
    };

    client::obelisk_client client2(connection);

//    libbitcoin::client::obelisk_client client {timeout_seconds, num_retries};

//    client.connect(address);
    if(!client2.connect(connection))
    {
        std::cout << "Fail" << std::endl;
    } else {
        std::cout << "Connection Succeeded" << std::endl;
    }

    client2.blockchain_fetch_last_height(on_error, on_reply);
    client2.wait();

	std::cout << "Hello bing\n";

}
