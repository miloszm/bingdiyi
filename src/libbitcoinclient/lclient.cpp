#include <iostream>
#include <bitcoin/system.hpp>
#include <bitcoin/client.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <nlohmann/json.hpp>


using namespace bc;
using namespace std;
using namespace boost::program_options;
using json = nlohmann::json;

/**
 * demonstrates programmatic call of a bx command
 */

int main(int argc, char *argv[]) {
    bool is_testnet{true};

    options_description desc(
            "Demonstrates programmatic call via libbitcoin client\n\nRequired options");
    desc.add_options()
            ("help,h", "print usage message")
            ("testnet,t", value<bool>(&is_testnet)->default_value(true),"use testnet blockchain");
    ;

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
        cout << "\n\n" << desc << "\n";
        cout << "example:"
             << "\n";
        cout
                << "--t=true"
                << "\n";
        return 1;
    }

    // note: must be after help option check
    notify(vm);

    std::ifstream ifs("config.json");
    json config_json = json::parse(ifs);
    string blockchain = is_testnet ? "testnet" : "mainnet";
    string libbitcoin_server_url = config_json[blockchain]["libbitcoin_connection"]["url"];

    client::connection_type connection = {};
    connection.retries = 3;
    connection.timeout_seconds = 8;
    connection.server = config::endpoint(libbitcoin_server_url);

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

    /**
     * broadcasting transaction
     * use with care
     */
//    chain::transaction tx;
//    data_chunk tx_chunk;
//    string tx_hex = "0100000001f1a134f2ff2e6e8ea9a66065f48c038355385be54a4f81785500715e4a8a2280010000006a4730440220423777a7f3484f700df4800bbf8e6949be074a01316bbf81edcc507a3c89b44c02207b8b9f37f2a8c4135cccdb9a909f0823c89d3e1876d2ed13fe0156c6a33ad94c012102817cb981372b3d3ec0422e0503b62c3acfc3aec2ef55a0d7f3ab5e1652814fd4feffffff0180380100000000001976a914a690b95933ddac2aaca68a4d067c5ba8e141faf788ac00000000";
//    if (!decode_base16(tx_chunk, tx_hex)){
//        cout << "could not decode from hex\n";
//    }
//
//    if (!tx.from_data(tx_chunk)){
//        cout << "could not decode transaction\n";
//    }
//    }
//    else {
//        client.transaction_pool_broadcast(on_error, on_error, tx);
//    }

}
