#include "src/common/bing_common.hpp"
#include "src/config/bing_config.hpp"
#include <bitcoin/system.hpp>
#include <boost/program_options.hpp>
#include <binglib/online_lock_tx_creator.hpp>
#include <binglib/purse_accessor.hpp>
#include <binglib/bing_wallet.hpp>
#include <binglib/wallet_state.hpp>
#include <binglib/history_inspector.hpp>
#include <algorithm>
#include "ronghua_client_provider.hpp"

using namespace boost::program_options;
using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;

int main() {
    RonghuaClient electrum_api_client;
    electrum_api_client.init(BingConfig::electrum_server_host,
                             BingConfig::electrum_server_service,
                             BingConfig::electrum_cert_file_path);

    bool is_testnet = true;
    int num_addresses0 = 50;
    int num_addresses1 = 50;
    vector<string> addresses;
    map<string, AddressDerivationResult> addresses_to_data;
    string seed_phrase =
        "effort canal zoo clown shoulder genuine penalty moral unit skate few quick";
    BingWallet::derive_electrum_addresses(is_testnet, seed_phrase,
                                          num_addresses0, num_addresses1,
                                          addresses, addresses_to_data);
    WalletState wallet_state(addresses, addresses_to_data);

    RonghuaClientProvider ronghua_client_provider(electrum_api_client);
    HistoryInspector history_inspector(is_testnet, ronghua_client_provider,
                                       wallet_state);

    string address {"n2z74VSLVyttZW3ayHCEdkYZ11nLNvoKDv"};

    cout << "calculating balance of address " << address << "\n";
    uint64_t balance = history_inspector.calculate_address_balance(address);

    cout << "balance of address " << address << " = " << balance << "\n";

    cout << "calculating wallet balance" << "\n";
    uint64_t total_balance = history_inspector.calculate_total_balance();

    cout << "wallet balance = " << total_balance << "\n";

    electrum_api_client.stop();
    return 0;
}
