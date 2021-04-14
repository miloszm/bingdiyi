#include "src/common/bing_common.hpp"
#include "src/config/bing_config.hpp"
#include <bitcoin/bitcoin.hpp>
#include <boost/program_options.hpp>
#include <binglib/online_lock_tx_creator.hpp>
#include <binglib/libb_client.hpp>
#include <binglib/electrum_api_client.hpp>
#include <binglib/purse_accessor.hpp>
#include <binglib/bing_wallet.hpp>
#include "src/walletstate/wallet_state.hpp"
#include "src/historyinspector/history_inspector.hpp"
#include <algorithm>

using namespace boost::program_options;
using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;




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

    WalletState wallet_state(addresses);

    HistoryInspector history_inspector(is_testnet, electrum_api_client, libb_client, wallet_state);

    uint64_t balance = history_inspector.calculate_address_balance("mkP2QQqQYsReSpt3JBoRQ5zVdw3ra1jenh");

    cout << "\n\nbalance=" << balance << "\n\n";

    uint64_t total_balance = history_inspector.calculate_total_balance();

    cout << "\n\ntotal_balance=" << total_balance << "\n\n";

    vector<HistoryViewRow> history_view_rows;
    history_inspector.create_history_view_rows(history_view_rows);

    for (auto& r: history_view_rows){
        cout << r.height << " " << r.balance_delta << " " << r.balance << " " << r.tx_id << "\n";
    }

    return 0;
}
