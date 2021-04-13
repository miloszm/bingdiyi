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


class WalletState {
public:
    WalletState(vector<string>& addresses);
    virtual ~WalletState();
    vector<string>& get_addresses();
    bool is_in_wallet(string address);
    transaction get_transaction(ElectrumApiClient &electrum_api_client, string txid);
    void print_cache();
    vector<transaction> get_all_tx_sorted(ElectrumApiClient &electrum_api_client);
    void add_to_all_history(const AddressHistoryItem& item);
private:
    vector<string> addresses_;
    map<string, string> tx_cache_;
    vector<AddressHistoryItem> all_history_;
private:
    static transaction hex_2_tx(string tx_hex);
};




#endif
