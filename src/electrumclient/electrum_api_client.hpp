#ifndef ELECTRUM_API_CLIENT_HPP
#define ELECTRUM_API_CLIENT_HPP

#include <string>
#include "electrum_client.hpp"


using namespace std;

struct AddressHistoryItem {
    string txid;
    int height;
};


void address_history_item_from_json(const nlohmann::json& j, AddressHistoryItem& ahi) {
    j.at("txid").get_to(ahi.txid);
    j.at("height").get_to(ahi.height);
}

typedef vector<AddressHistoryItem> AddressHistory;

void address_history_from_json(const nlohmann::json& j, AddressHistory& ah){
    auto items = j.items();
    for (auto i = items.begin(); i != items.end(); ++i){
        AddressHistoryItem ahi;
        address_history_item_from_json(i.value(), ahi);
        ah.push_back(ahi);
    }
}

class ElectrumApiClient {
public:
    ElectrumApiClient(ElectrumClient& client): client_(client){}

    vector<AddressHistory> getHistory(vector<string> addresses);
private:
    ElectrumClient& client_;
    atomic<int> id_counter;
};

#endif