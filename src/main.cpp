#include <iostream>
//#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client/obelisk_client.hpp>


int main() {
    static constexpr uint8_t num_retries = 0;
    static constexpr uint16_t timeout_seconds = 8;

    libbitcoin::client::obelisk_client client {timeout_seconds, num_retries};

	std::cout << "Hello bing\n";

}
