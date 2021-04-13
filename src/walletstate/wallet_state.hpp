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
    transaction get_transaction(ElectrumApiClient &electrum_api_client, string txid);
private:
    vector<string> addresses_;
    map<string, string> tx_cache_;
private:
    static transaction hex_2_tx(string tx_hex);
};




#endif
