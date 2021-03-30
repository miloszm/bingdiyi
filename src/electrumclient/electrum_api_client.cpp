#include "electrum_api_client.hpp"

using json = nlohmann::json;
using namespace std;

vector<AddressHistory> ElectrumApiClient::getHistory(vector<string> addresses){
    vector<AddressHistory> address_histories;
    for (string address: addresses) {
        ElectrumRequest request{"blockchain.scripthash.get_history", ++id_counter, "["+address+"]"};
        json json_request;
        electrum_request_to_json(json_request, request);
        json json_response = client_.send_request(json_request);
        AddressHistory address_history;
        address_history_from_json(json_response["result"], address_history);
        address_histories.push_back(address_history);
    }
    return address_histories;
}
