#ifndef ELECTRUM_API_CLIENT_HPP
#define ELECTRUM_API_CLIENT_HPP

#include <string>
#include "electrum_client.hpp"


using namespace std;

struct AddressHistoryItem {
    string txid;
    int height;
};


void address_history_item_from_json(const nlohmann::json& j, AddressHistoryItem& ahi);

typedef vector<AddressHistoryItem> AddressHistory;

void address_history_from_json(const nlohmann::json& j, AddressHistory& ah);

class ElectrumApiClient {
public:
    ElectrumApiClient(ElectrumClient& client): client_(client){}

    vector<AddressHistory> getHistory(vector<string> addresses);
private:
    ElectrumClient& client_;
    atomic<int> id_counter;
};

#endif