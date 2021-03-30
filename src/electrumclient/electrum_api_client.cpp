#include <iostream>
#include "electrum_api_client.hpp"

using json = nlohmann::json;
using namespace std;


void address_history_item_from_json(const nlohmann::json& j, AddressHistoryItem& ahi) {
    j.at("tx_hash").get_to(ahi.txid);
    j.at("height").get_to(ahi.height);
}


void address_history_from_json(const nlohmann::json& j, AddressHistory& ah){
    auto items = j.items();
    for (auto i = items.begin(); i != items.end(); ++i){
        AddressHistoryItem ahi;
        address_history_item_from_json(i.value(), ahi);
        ah.push_back(ahi);
    }
}


void address_balance_from_json(const nlohmann::json& j, AddressBalance& ab) {
    j.at("confirmed").get_to(ab.confirmed);
    j.at("unconfirmed").get_to(ab.unconfirmed);
}


vector<AddressHistory> ElectrumApiClient::getHistory(vector<string> addresses){
    vector<AddressHistory> address_histories;
    for (string address: addresses) {
        vector<string> av{address};
        ElectrumRequest request{"blockchain.scripthash.get_history", ++id_counter, av};
        json json_request;
        electrum_request_to_json(json_request, request);
        json json_response = client_.send_request(json_request);
        AddressHistory address_history;
        address_history_from_json(json_response["result"], address_history);
        address_histories.push_back(address_history);
    }
    return address_histories;
}


AddressBalance ElectrumApiClient::getBalance(string address){
    vector<string> av{address};
    ElectrumRequest request{"blockchain.scripthash.get_balance", ++id_counter, av};
    json json_request;
    electrum_request_to_json(json_request, request);
    json json_response = client_.send_request(json_request);
    AddressBalance address_balance;
    address_balance_from_json(json_response["result"], address_balance);
    return address_balance;
}
