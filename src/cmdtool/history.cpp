#include "src/common/bing_common.hpp"
#include "src/config/bing_config.hpp"
#include <bitcoin/bitcoin.hpp>
#include <boost/program_options.hpp>
#include <binglib/online_lock_tx_creator.hpp>
#include <binglib/libb_client.hpp>
#include <binglib/electrum_api_client.hpp>
#include <binglib/purse_accessor.hpp>
#include <binglib/bing_wallet.hpp>
#include <algorithm>

using namespace boost::program_options;
using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;



chain::transaction hex_2_tx(string tx_hex){
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


chain::transaction fetch_tx(ElectrumApiClient &electrum_api_client, string txid){
    string tx_hex = electrum_api_client.getTransaction(txid);
    return hex_2_tx(tx_hex);
}


void analyse_tx(ElectrumApiClient &electrum_api_client, string tx_hex, vector<string>& addresses){
    chain::transaction tx = hex_2_tx(tx_hex);

    cout << "total input value: " << tx.total_input_value() << "\n";
    cout << "total output value: " << tx.total_output_value() << "\n";
    for (auto& i: tx.inputs()){
        string funding_tx = encode_hash(i.previous_output().hash());
        int funding_idx = i.previous_output().index();
        chain::transaction input_tx = fetch_tx(electrum_api_client, funding_tx);
        auto& ii = input_tx.outputs().at(funding_idx);
        cout << "input from utxo: " << funding_tx << ":" << funding_idx << "\n";
        cout << "input value: " << ii.value() << "\n";
    };
    for (auto& o: tx.outputs()){
        wallet::payment_address::list axx = o.addresses(wallet::payment_address::testnet_p2kh, wallet::payment_address::testnet_p2sh);
        if (axx.size() > 0) {
            for (wallet::payment_address& ax: axx) {
                cout << "output value: " << o.value() << " address: " << ax << " script: "
                     << static_cast<int>(o.script().pattern()) << "\n";
                bool is_in_wallet = (std::find(addresses.begin(), addresses.end(), ax.encoded()) != addresses.end());
                cout << "in wallet=" << is_in_wallet << "\n";
            }
        } else {
            cout << "output value: " << o.value() << "\n";
        }
    };

}



int main() {
    LibbClient libb_client;
    libb_client.init(BingConfig::libbitcoin_server_url);
    ElectrumClient electrum_client;
    electrum_client.init(BingConfig::electrum_server_host, BingConfig::electrum_server_service, BingConfig::electrum_cert_file_path);
    ElectrumApiClient electrum_api_client(electrum_client);

    bool is_testnet = true;
    int  num_addresses0 = 51;
    int  num_addresses1 = 15;
    vector<string> addresses;
    map<string, AddressDerivationResult> addresses_to_data;
    string seed_phrase = "effort canal zoo clown shoulder genuine penalty moral unit skate few quick";
    BingWallet::derive_electrum_addresses(is_testnet, seed_phrase, num_addresses0, num_addresses1, addresses, addresses_to_data);

    vector<HistoryItem> history_items;

    vector<string> single_address{"mihBbdmqPut61bs9eDYZ3fdYxAKEP3mdiX"};
    PurseAccessor::find_history(electrum_api_client, libb_client, single_address, history_items);

    for (HistoryItem& item: history_items){
        cout << "=============================\n";
        cout << item.txid << " " << item.address << " " << item.height << " " << item.txhex << "\n";
        analyse_tx(electrum_api_client, item.txhex, addresses);
        cout << "=============================\n\n";
    }

    return 0;
}
