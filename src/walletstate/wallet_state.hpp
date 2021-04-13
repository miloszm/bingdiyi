#ifndef WALLET_STATE_HPP
#define WALLET_STATE_HPP

#include <bitcoin/bitcoin.hpp>
#include <binglib/electrum_api_client.hpp>

using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;
#include <string>

using namespace std;


struct TransactionAndHeight {
    transaction tx;
    int height;
};

class WalletState {
public:
    WalletState(vector<string>& addresses);
    virtual ~WalletState();
    vector<string>& get_addresses();
    bool is_in_wallet(string address);
    transaction get_transaction(ElectrumApiClient &electrum_api_client, string txid);
    void get_history(ElectrumApiClient &electrum_api_client, const string& address, vector<AddressHistoryItem>& history_items);
    vector<TransactionAndHeight> get_all_txs_sorted(ElectrumApiClient &electrum_api_client);
private:
    vector<string> addresses_;
    map<string, string> tx_cache_;
    map<string, vector<AddressHistoryItem>> address_history_cache_;
    vector<AddressHistoryItem> all_history_;
private:
    static transaction hex_2_tx(string tx_hex);
    void refresh_all_history(ElectrumApiClient &electrum_api_client);
};




#endif
