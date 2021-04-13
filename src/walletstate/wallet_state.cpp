#include <binglib/bing_common.hpp>
#include "wallet_state.hpp"
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

void WalletState::print_cache() {
    for (auto e: tx_cache_){
        cout << e.first << " -> " << e.second << "\n";
    }
}

vector<transaction> WalletState::get_all_tx_sorted(ElectrumApiClient &electrum_api_client) {
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
        try {
            transaction tx = get_transaction(electrum_api_client, item.txid);
            txs.push_back(tx);
        }
        catch(exception& e) {
            cerr << e.what() << "\n";
        }
    }

    return txs;
}

void WalletState::add_to_all_history(const AddressHistoryItem& item) {
    all_history_.push_back(item);
}
