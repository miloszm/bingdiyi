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

//========

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

//========

void analyse_tx(ElectrumApiClient &electrum_api_client, string tx_id, string tx_hex, vector<string>& addresses, vector<TxBalance>& balance_items){
    chain::transaction tx = hex_2_tx(tx_hex);
    cout << "tot out: " << tx.total_output_value() << "\n";
    vector<TxBalanceInput> balance_inputs;
    vector<TxBalanceOutput> balance_outputs;
    for (auto& i: tx.inputs()){
        string funding_tx = encode_hash(i.previous_output().hash());
        int funding_idx = i.previous_output().index();
        chain::transaction input_tx = fetch_tx(electrum_api_client, funding_tx);
        auto& ii = input_tx.outputs().at(funding_idx);
        cout << "   input from utxo: " << funding_tx << ":" << funding_idx << " value: " << ii.value() << "\n";
        TxBalanceInput balance_input {funding_tx, funding_idx, ii.value()};
        balance_inputs.push_back(balance_input);
    };
    for (auto& o: tx.outputs()){
        wallet::payment_address::list axx = o.addresses(wallet::payment_address::testnet_p2kh, wallet::payment_address::testnet_p2sh);
        if (axx.size() > 0) {
            for (wallet::payment_address& ax: axx) {
                bool is_in_wallet = (std::find(addresses.begin(), addresses.end(), ax.encoded()) != addresses.end());
                cout << "   output value: " << o.value() << " address: " << ax << " script: "
                     << static_cast<int>(o.script().pattern()) << " in wallet=" << is_in_wallet << "\n";
                TxBalanceOutput balance_output {ax.encoded(), static_cast<int>(o.script().pattern()), o.value(), is_in_wallet};
                balance_outputs.push_back(balance_output);
                break;
            }
        } else {
            cout << "   output value: " << o.value() << " in wallet=0" << "\n";
            TxBalanceOutput balance_output {"", -1, o.value(), false};
            balance_outputs.push_back(balance_output);
        }
    };
    TxBalance tx_balance{tx_id, balance_inputs, balance_outputs};
    balance_items.push_back(tx_balance);
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

    vector<string> single_address{"mvhfqVtVytsTn1LzZpJ7ura59HSPGbPpRP"};
    PurseAccessor::find_history(electrum_api_client, libb_client, single_address, history_items);

    vector<TxBalance> balance_items;

    for (HistoryItem& item: history_items){
        cout << item.txid << " " << item.address << " ";
        analyse_tx(electrum_api_client, item.txid, item.txhex, addresses, balance_items);
    }

    // calc address balance (single_address)
    uint64_t balance{0};
    int cur_pos{0};
    for (TxBalance& balance_item: balance_items){
        for (int oidx = 0; oidx < balance_item.outputs.size(); ++ oidx){
            TxBalanceOutput &o = balance_item.outputs[oidx];
            if (o.address == single_address.at(0)){
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

    cout << "balance=" << balance << "\n";

    return 0;
}
