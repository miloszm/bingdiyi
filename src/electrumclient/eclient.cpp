#include <iostream>
#include <binglib/address_converter.hpp>
#include <binglib/ronghua_client.hpp>
#include <boost/program_options.hpp>
#include <fstream>

using json = nlohmann::json;
using namespace std;
using namespace boost::program_options;

int main(int argc, char *argv[]) {
    try {
        bool is_testnet{true};
        string address{""};

        options_description desc(
            "Demonstrates examples of programmatic calls via Electrum client\n\nRequired options");
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
        string electrum_server_host = config_json[blockchain]["electrum_connection"]["host"];
        string electrum_server_service = config_json[blockchain]["electrum_connection"]["service"];
        string electrum_cert_file_path = config_json[blockchain]["electrum_connection"]["cert_file_path"];

        RonghuaClient electrum_api_client;
        electrum_api_client.init(electrum_server_host, electrum_server_service, electrum_cert_file_path);

        json banner_request =
            R"({"jsonrpc":"2.0","method":"server.banner","id":1712})"_json;
        cout << "sending direct - server banner request..." << "\n";
        json response = electrum_api_client.send_request(banner_request, 1712);
        cout << "response = \n";
        cout << response.dump(4) << "\n\n";

        json get_history_request =
            R"({"jsonrpc":"2.0","method":"blockchain.scripthash.get_history","id":1713,"params":["af89af88915ddf9ee02b223800d66aec14e01bb523bd870c6c358fb935d9f004"]})"_json;
        cout << "sending direct - get history request..." << "\n";
        response = electrum_api_client.send_request(get_history_request, 1713);
        cout << "response = \n";
        cout << response.dump(4) << "\n\n";

        json get_balance_request =
            R"({"jsonrpc":"2.0","method":"blockchain.scripthash.get_balance","id":1714,"params":["af89af88915ddf9ee02b223800d66aec14e01bb523bd870c6c358fb935d9f004"]})"_json;
        cout << "sending direct - get balance request..." << "\n";
        response = electrum_api_client.send_request(get_balance_request, 1714);
        cout << "response = \n";
        cout << response.dump(4) << "\n\n";

        cout << "sending via api - get history request..." << "\n";
        string addr{AddressConverter::base58_to_spkh_hex(
            "mpS14bFCZiHFRxfNNbnPT2FScJBrm96iLE")};
        AddressHistory history = electrum_api_client.getHistory(addr);
        for (auto& h: history) {
            cout << h.height << " " << h.txid << "\n\n";
        }

        cout << "sending via api - get balance request..." << "\n";
        AddressBalance ab = electrum_api_client.getBalance(addr);
        cout << "mpS14bFCZiHFRxfNNbnPT2FScJBrm96iLE balance = " << ab.confirmed
             << " confirmed, " << ab.unconfirmed << " unconfirmed \n\n";
        electrum_api_client.stop();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
