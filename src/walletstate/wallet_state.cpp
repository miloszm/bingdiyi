#include <binglib/bing_common.hpp>
#include "wallet_state.hpp"

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
