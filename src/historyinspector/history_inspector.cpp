#include <binglib/bing_common.hpp>
#include "history_inspector.hpp"
#include <binglib/address_converter.hpp>

using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;

HistoryInspector::HistoryInspector(bool is_testnet,
                                   ElectrumApiClient &electrum_api_client,
                                   LibbClient &libb_client,
                                   WalletState &wallet_state)
    : is_testnet_(is_testnet), electrum_api_client_(electrum_api_client),
      libb_client_(libb_client), wallet_state_(wallet_state) {}

HistoryInspector::~HistoryInspector() {}

uint64_t HistoryInspector::calculate_total_balance() {
    uint64_t balance{0};
    for (const string &address : wallet_state_.get_addresses()) {
        balance += calculate_address_balance(address);
    }
    return balance;
}

uint64_t HistoryInspector::calculate_address_balance(const string &address) {
    vector<AddressHistoryItem> history_items;

    wallet_state_.get_history(electrum_api_client_, address, history_items);

    vector<TxBalance> balance_items;

    for (AddressHistoryItem &item : history_items) {
        analyse_tx_balances(item.txid, balance_items);
    }

    uint64_t balance = calc_address_balance(address, balance_items);

    return balance;
}

wallet::payment_address::list HistoryInspector::get_addresses(output &o) {
    if (is_testnet_) {
        return o.addresses(wallet::payment_address::testnet_p2kh,
                           wallet::payment_address::testnet_p2sh);
    } else {
        return o.addresses(wallet::payment_address::mainnet_p2kh,
                           wallet::payment_address::mainnet_p2sh);
    }
}

void HistoryInspector::analyse_tx_balances(string tx_id,
                                           vector<TxBalance> &balance_items) {
    chain::transaction tx =
        wallet_state_.get_transaction(electrum_api_client_, tx_id);
    vector<TxBalanceInput> balance_inputs;
    vector<TxBalanceOutput> balance_outputs;
    for (auto &i : tx.inputs()) {
        string funding_tx_id = encode_hash(i.previous_output().hash());
        int funding_idx = i.previous_output().index();
        chain::transaction funding_tx =
            wallet_state_.get_transaction(electrum_api_client_, funding_tx_id);
        auto &previous_output = funding_tx.outputs().at(funding_idx);
        wallet::payment_address::list axx = get_addresses(previous_output);
        if (axx.size() > 0) {
            wallet::payment_address &ax = axx.front();
            bool is_in_wallet = wallet_state_.is_in_wallet(ax.encoded());
            TxBalanceInput balance_input{funding_tx_id, funding_idx,
                                         previous_output.value(), is_in_wallet};
            balance_inputs.push_back(balance_input);
        } else {
            TxBalanceInput balance_input{funding_tx_id, funding_idx,
                                         previous_output.value(), false};
            balance_inputs.push_back(balance_input);
        }
    };
    for (auto &o : tx.outputs()) {
        wallet::payment_address::list axx = get_addresses(o);
        if (axx.size() > 0) {
            wallet::payment_address &ax = axx.front();
            bool is_in_wallet = wallet_state_.is_in_wallet(ax.encoded());
            TxBalanceOutput balance_output{
                ax.encoded(), static_cast<int>(o.script().pattern()), o.value(),
                is_in_wallet};
            balance_outputs.push_back(balance_output);
        } else {
            TxBalanceOutput balance_output{"", -1, o.value(), false};
            balance_outputs.push_back(balance_output);
        }
    };
    TxBalance tx_balance{tx_id, balance_inputs, balance_outputs};
    balance_items.push_back(tx_balance);
}

/**
 * note: balance_items have to be in order of real time appearance of corresponding transactions
 */
uint64_t
HistoryInspector::calc_address_balance(const string &address,
                                       vector<TxBalance> &balance_items) {
    uint64_t balance{0};
    int cur_pos{0};
    for (TxBalance &balance_item : balance_items) {
        for (int oidx = 0; oidx < balance_item.outputs.size(); ++oidx) {
            TxBalanceOutput &o = balance_item.outputs[oidx];
            if (o.address == address) {
                balance += o.value;
                string cur_tx = balance_item.tx_id;
                int cur_idx = oidx;
                for (auto j = cur_pos + 1; j < balance_items.size(); ++j) {
                    TxBalance &balance_item2 = balance_items[j];
                    for (TxBalanceInput &i : balance_item2.inputs) {
                        if (i.funding_tx == cur_tx &&
                            i.funding_idx == cur_idx) {
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

int64_t HistoryInspector::calculate_tx_wallet_impact(const string &tx_id) {
    vector<TxBalance> balance_items;
    analyse_tx_balances(tx_id, balance_items);
    uint64_t sum_from_wallet_inputs{0};
    uint64_t sum_to_wallet_outputs{0};
    for (const TxBalance &balance_item : balance_items) {
        for (const TxBalanceInput &i : balance_item.inputs) {
            bool inside = i.in_wallet;
            if (inside)
                sum_from_wallet_inputs += i.value;
        }
        for (const TxBalanceOutput &o : balance_item.outputs) {
            bool inside = o.in_wallet;
            if (inside)
                sum_to_wallet_outputs += o.value;
        }
    }
    return sum_to_wallet_outputs - sum_from_wallet_inputs;
}

void HistoryInspector::create_history_view_rows(
    vector<HistoryViewRow> &history_view_rows) {

    vector<TransactionAndHeight> sorted_txs =
        wallet_state_.get_all_txs_sorted(electrum_api_client_);

    for (auto &tx_and_height : sorted_txs) {
        auto &tx = tx_and_height.tx;
        string tx_id = encode_hash(tx.hash());
        int64_t impact = calculate_tx_wallet_impact(tx_id);
        // todo: cache it
        // for the time being I skip timestamp as it takes too long
        // let's see if there is some quicker way to obtain it
        //        string header_hex = electrum_api_client_.getBlockHeader(tx_and_height.height);
        //        chain::header block_header = hex_2_header(header_hex);
        //        uint32_t timestamp = block_header.timestamp();
        uint32_t timestamp = 0;
        HistoryViewRow history_view_row{timestamp, tx_and_height.height, impact,
                                        tx_id, 0};
        history_view_rows.push_back(history_view_row);
    }
    uint64_t balance{0};
    for (auto p = history_view_rows.rbegin(); p != history_view_rows.rend();
         ++p) {
        balance += p->balance_delta;
        p->balance = balance;
    }
}

chain::header HistoryInspector::hex_2_header(string header_hex) {
    chain::header header;
    data_chunk header_chunk;

    if (!decode_base16(header_chunk, header_hex)) {
        throw std::invalid_argument("could not decode raw hex header");
    }

    if (!header.from_data(header_chunk)) {
        throw std::invalid_argument("could not decode header");
    }

    return header;
}
