#include <iostream>
#include "electrum_api_client.hpp"
#include <binglib/address_converter.hpp>
#include "src/config/bing_config.hpp"

using json = nlohmann::json;
using namespace std;

int main(int argc, char *argv[]) {
  try {
    ElectrumClient electrum_client;

    electrum_client.init(BingConfig::electrum_server_host, BingConfig::electrum_server_service, BingConfig::electrum_cert_file_path);

//    json banner_request =
//        R"({"jsonrpc":"2.0","method":"server.banner","id":1712})"_json;
//    json get_history_request =
//        R"({"jsonrpc":"2.0","method":"blockchain.scripthash.get_history","id":1712,"params":["af89af88915ddf9ee02b223800d66aec14e01bb523bd870c6c358fb935d9f004"]})"_json;
//    json get_balance_request =
//        R"({"jsonrpc":"2.0","method":"blockchain.scripthash.get_balance","id":1712,"params":["af89af88915ddf9ee02b223800d66aec14e01bb523bd870c6c358fb935d9f004"]})"_json;
//
//
//    cout << "sending request..."
//              << "\n";
//    json response = electrum_client.send_request(get_balance_request);
//    cout << "response = \n";
//    cout << response.dump(4) << "\n";

    cout << "========================== \n";

    ElectrumApiClient electrum_api_client(electrum_client);
    string addr {AddressConverter::base58_to_spkh_hex("mpS14bFCZiHFRxfNNbnPT2FScJBrm96iLE")};
//    vector<string> addresses{addr};
//    vector<AddressHistory> histories = electrum_api_client.getHistory(addresses);
//    cout << "got " << histories.size() << " histories" << "\n";
//    auto hh = histories.at(0);
//    for (auto h: hh) {
//        cout << h.height << " " << h.txid << "\n";
//    }

    AddressBalance ab = electrum_api_client.getBalance(addr);
    cout << "balance = " << ab.confirmed << " confirmed, " << ab.unconfirmed << " unconfirmed \n\n";

  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
