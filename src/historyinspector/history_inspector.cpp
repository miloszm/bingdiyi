#include <binglib/bing_common.hpp>
#include "history_inspector.hpp"
#include <binglib/address_converter.hpp>

using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;

HistoryInspector::HistoryInspector(ElectrumApiClient &electrum_api_client, WalletState& wallet_state)
    : electrum_api_client_(electrum_api_client), wallet_state_(wallet_state){}

HistoryInspector::~HistoryInspector(){}

uint64_t HistoryInspector::calculate_total_balance(){
    return 0;
}

uint64_t HistoryInspector::calculate_address_balance(const string& address){
    vector<ElectrumHistoryItem> history_items;

    find_history(electrum_api_client_, address, history_items);

    vector<TxBalance> balance_items;

    for (ElectrumHistoryItem& item: history_items){
        analyse_tx_balances(item.txid, balance_items);
    }

    uint64_t balance = calc_address_balance(address, balance_items);

    return balance;
}

void HistoryInspector::find_history(ElectrumApiClient &electrum_api_client, const string& address, vector<ElectrumHistoryItem>& history_items){
    string address_spkh = AddressConverter::base58_to_spkh_hex(address);
    AddressHistory history = electrum_api_client.getHistory(address_spkh);
    for (const AddressHistoryItem& history_item: history){
        ElectrumHistoryItem item;
        item.txid = history_item.txid;
        item.height = history_item.height;
        history_items.push_back(item);
    }
}

void HistoryInspector::analyse_tx_balances(string tx_id, vector<TxBalance>& balance_items){
    chain::transaction tx = wallet_state_.get_transaction(electrum_api_client_, tx_id);
    vector<TxBalanceInput> balance_inputs;
    vector<TxBalanceOutput> balance_outputs;
    for (auto& i: tx.inputs()){
        string funding_tx_id = encode_hash(i.previous_output().hash());
        int funding_idx = i.previous_output().index();
        chain::transaction funding_tx = wallet_state_.get_transaction(electrum_api_client_, funding_tx_id);
        auto& previous_output = funding_tx.outputs().at(funding_idx);
        TxBalanceInput balance_input {funding_tx_id, funding_idx, previous_output.value()};
        balance_inputs.push_back(balance_input);
    };
    for (auto& o: tx.outputs()){
        wallet::payment_address::list axx = o.addresses(wallet::payment_address::testnet_p2kh, wallet::payment_address::testnet_p2sh);
        if (axx.size() > 0) {
            for (wallet::payment_address& ax: axx) {
                bool is_in_wallet = wallet_state_.is_in_wallet(ax.encoded());
                TxBalanceOutput balance_output {ax.encoded(), static_cast<int>(o.script().pattern()), o.value(), is_in_wallet};
                balance_outputs.push_back(balance_output);
                break;
            }
        } else {
            TxBalanceOutput balance_output {"", -1, o.value(), false};
            balance_outputs.push_back(balance_output);
        }
    };
    TxBalance tx_balance{tx_id, balance_inputs, balance_outputs};
    balance_items.push_back(tx_balance);
}

/**
 * note: balance_items have to be in order of real time appearance of corresponding transactions
 */
uint64_t HistoryInspector::calc_address_balance(const string& address, vector<TxBalance>& balance_items){
    uint64_t balance{0};
    int cur_pos{0};
    for (TxBalance& balance_item: balance_items){
        for (int oidx = 0; oidx < balance_item.outputs.size(); ++ oidx){
            TxBalanceOutput &o = balance_item.outputs[oidx];
            if (o.address == address){
                balance += o.value;
                string cur_tx = balance_item.tx_id;
                int cur_idx = oidx;
                for (auto j = cur_pos+1; j < balance_items.size(); ++j){
                    TxBalance& balance_item2 = balance_items[j];
                    for (TxBalanceInput &i: balance_item2.inputs){
                        if (i.funding_tx == cur_tx && i.funding_idx == cur_idx){
                            balance -= i.value;
                        }
                    }
                }
            }
        }
        ++cur_pos;
    }
    return balance;
}
