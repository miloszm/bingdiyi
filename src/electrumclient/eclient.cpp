#include <iostream>
#include "electrum_api_client.hpp"

using json = nlohmann::json;
using namespace std;

int main(int argc, char *argv[]) {
  try {
    //  ("localhost", "51002");
    //  ("blockstream.info", "993");
    //  ("testnet.qtornado.com", "51002");
    //  ("testnet.electrumx.hodlwallet.com", "51002");

    ElectrumClient electrum_client;

    electrum_client.init("localhost", "51002", "cert.crt");

//    json banner_request =
//        R"({"jsonrpc":"2.0","method":"server.banner","id":1712})"_json;
//    json get_history_request =
//        R"({"jsonrpc":"2.0","method":"blockchain.scripthash.get_history","id":1712,"params":["af89af88915ddf9ee02b223800d66aec14e01bb523bd870c6c358fb935d9f004"]})"_json;
//
//    cout << "sending request..."
//              << "\n";
//    json response = electrum_client.send_request(get_history_request);
//            c.send_request(banner_request);
//    cout << "response = \n";
//    cout << response.dump(4) << "\n";

    cout << "========================== \n";

    ElectrumApiClient electrum_api_client(electrum_client);
    vector<string> addresses{"af89af88915ddf9ee02b223800d66aec14e01bb523bd870c6c358fb935d9f004"};
    vector<AddressHistory> histories = electrum_api_client.getHistory(addresses);
    cout << "got " << histories.size() << " histories" << "\n";
    auto hh = histories.at(0);
    for (auto h: hh) {
        cout << h.height << " " << h.txid << "\n";
    }

  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
