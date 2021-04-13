#include <binglib/bing_common.hpp>
#include "wallet_state.hpp"
#include <binglib/address_converter.hpp>
#include <algorithm>

using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;


WalletState::WalletState(vector<string>& addresses): addresses_(addresses){}

WalletState::~WalletState(){}

vector<string>& WalletState::get_addresses() {
    return addresses_;
}

bool WalletState::is_in_wallet(string address) {
    return std::find(addresses_.begin(), addresses_.end(), address) != addresses_.end();
}


chain::transaction WalletState::hex_2_tx(string tx_hex){
    chain::transaction tx;
    data_chunk tx_chunk;

    if (!decode_base16(tx_chunk, tx_hex)){
        throw std::invalid_argument("could not decode raw hex transaction");
    }

    if (!tx.from_data(tx_chunk)){
        throw std::invalid_argument("could not decode transaction");
    }

    return tx;
}

chain::transaction WalletState::get_transaction(ElectrumApiClient &electrum_api_client, string txid){
    string tx_hex = tx_cache_[txid];
    if (tx_hex.empty()){
        tx_hex = electrum_api_client.getTransaction(txid);
        tx_cache_[txid] = tx_hex;
    }
    return hex_2_tx(tx_hex);
}

void WalletState::get_history(ElectrumApiClient &electrum_api_client, const string& address, vector<AddressHistoryItem>& history_items){
    vector<AddressHistoryItem> address_history = address_history_cache_[address];
    if(address_history.empty()){
        string address_spkh = AddressConverter::base58_to_spkh_hex(address);
        AddressHistory history = electrum_api_client.getHistory(address_spkh);
        for (const AddressHistoryItem& history_item: history){
            history_items.push_back(history_item);
        }
        address_history_cache_[address] = history_items;
    } else {
        for (const AddressHistoryItem& history_item: address_history){
            history_items.push_back(history_item);
        }
    }
}


void WalletState::refresh_all_history(ElectrumApiClient &electrum_api_client) {
    all_history_.clear();
    for (auto& address: addresses_){
        vector<AddressHistoryItem> history_items;
        get_history(electrum_api_client, address, history_items);
        for (auto& history_item: history_items){
            all_history_.push_back(history_item);
        }
    }
}


vector<transaction> WalletState::get_all_txs_sorted(ElectrumApiClient &electrum_api_client) {
    refresh_all_history(electrum_api_client);
    std::sort( all_history_.begin( ), all_history_.end( ), [ ]( const AddressHistoryItem& lhs, const AddressHistoryItem& rhs )
    {
        if (lhs.height != rhs.height)
            return lhs.height > rhs.height;
        else
            return lhs.txid > rhs.txid;
    });
    auto last = std::unique( all_history_.begin(), all_history_.end(), [ ]( const AddressHistoryItem& lhs, const AddressHistoryItem& rhs )
    {
        return lhs.txid == rhs.txid;
    });
    all_history_.erase( last, all_history_.end() );

    vector<transaction> txs;
    for (const AddressHistoryItem& item: all_history_){
        transaction tx = get_transaction(electrum_api_client, item.txid);
        txs.push_back(tx);
    }

    return txs;
}
