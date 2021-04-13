#ifndef BINGDIYI_HISTORY_INSPECTOR_HPP
#define BINGDIYI_HISTORY_INSPECTOR_HPP

#include <bitcoin/bitcoin.hpp>
#include "src/walletstate/wallet_state.hpp"

using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;
#include <string>

using namespace std;


struct TxBalanceInput {
    string funding_tx;
    int funding_idx;
    uint64_t value;
};

struct TxBalanceOutput {
    string address;
    int script_kind;
    uint64_t value;
    bool in_wallet;
};


struct TxBalance {
    string tx_id;
    vector<TxBalanceInput> inputs;
    vector<TxBalanceOutput> outputs;
};


struct ElectrumHistoryItem {
    string txid;
    int height;
};


class HistoryInspector {
public:
    HistoryInspector(ElectrumApiClient &electrum_api_client, WalletState& wallet_state);
    virtual ~HistoryInspector();

    uint64_t calculate_address_balance(const string& address);
    uint64_t calculate_total_balance();


private:
    WalletState& wallet_state_;
    ElectrumApiClient &electrum_api_client_;

private:
    static void find_history(ElectrumApiClient &electrum_api_client, const string& address, vector<ElectrumHistoryItem>& history_items);
    void analyse_tx_balances(string tx_id, vector<TxBalance>& balance_items);
    static uint64_t calc_address_balance(const string& address, vector<TxBalance>& balance_items);
};



#endif
