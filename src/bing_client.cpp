#include "bing_client.hpp"


using namespace bc::system;


void BingClient::init(){
    connection = {};
    connection.retries = 3;
    //connection.server = config::endpoint("tcp://mainnet.libbitcoin.net:9091");
    //connection.server = config::endpoint("tcp://mainnet2.libbitcoin.net:9091");
    connection.server = libbitcoin::system::config::endpoint("tcp://testnet1.libbitcoin.net:19091");
}

void BingClient::doConnect(client::obelisk_client& client){
    if(!client.connect(connection))
    {
        std::cout << "Connection Failed" << std::endl;
    }
}

size_t BingClient::fetchHeight(){

    client::obelisk_client client;
    doConnect(client);

    size_t height;

//    const auto on_error = [](const code& ec)
//    {
//        std::cout << "Error Code: " << ec.message() << std::endl;
//    };

    auto on_reply = [&height](const system::code&, size_t blockHeight)
    {
        height = blockHeight;
    };

    client.blockchain_fetch_last_height(on_reply);
    client.wait();
    return height;
}

system::chain::points_value BingClient::fetchUtxo(const wallet::payment_address address, uint64_t satoshis, wallet::select_outputs::algorithm algorithm){

    client::obelisk_client client;
    doConnect(client);

    system::chain::points_value pointsValue;

//    const auto on_error = [](const code& ec)
//    {
//        std::cout << "Error Code: " << ec.message() << std::endl;
//    };

    auto on_reply = [&pointsValue](const system::code&, const system::chain::points_value& pv)
    {
        pointsValue = pv;
    };

    client.blockchain_fetch_unspent_outputs(on_reply, address, satoshis, algorithm);
    client.wait();

    return pointsValue;

};
