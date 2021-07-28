#include "src/common/bing_common.hpp"
#include <bitcoin/system.hpp>
#include <boost/program_options.hpp>
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

int main(int argc, char* argv[]) {
    try {
        const int DEFAULT_NUM_RCV_ADDRESSES {50};
        const int DEFAULT_NUM_CHG_ADDRESSES {50};
        int num_rcv_addresses {DEFAULT_NUM_RCV_ADDRESSES};
        int num_chg_addresses {DEFAULT_NUM_CHG_ADDRESSES};
        string seed_phrase{""};
        bool is_testnet {true};
        options_description desc("Displays wallet history\n\nRequired options");
        desc.add_options()
                ("help,h", "print usage message")
                ("seed,s", value<string>(&seed_phrase)->required(), "Electrum seed phrase")
                ("receiving-addresses,r", value<int>(&num_rcv_addresses)->default_value(DEFAULT_NUM_RCV_ADDRESSES), "number of receiving addresses")
                ("change-addresses,c", value<int>(&num_chg_addresses)->default_value(DEFAULT_NUM_CHG_ADDRESSES),"number of change addresses")
                ("testnet,t", value<bool>(&is_testnet)->default_value(true),"use testnet blockchain");
                ;

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);

        if (vm.count("help") || argc <= 1) {
            cout << "\n\n" << desc << "\n";
            cout << "example:" << "\n";
            cout
                    << "--r=50 --c=50 --s=""\"effort canal zoo clown shoulder genuine penalty moral unit skate few quick\" --t=true"
                    << "\n";
            return 1;
        }

        // note: must be after help option check
        notify(vm);

        std::ifstream ifs("config.json");
        json config_json = json::parse(ifs);
        string blockchain = is_testnet ? "testnet" : "mainnet";
        string electrum_server_host = config_json[blockchain]["electrum_connection"]["host"];
        string electrum_server_service = config_json[blockchain]["electrum_connection"]["service"];
        string electrum_cert_file_path = config_json[blockchain]["electrum_connection"]["cert_file_path"];

        RonghuaClient electrum_api_client;
        electrum_api_client.init(electrum_server_host,
                                 electrum_server_service,
                                 electrum_cert_file_path);

        vector<string> addresses;
        map<string, AddressDerivationResult> addresses_to_data;
        BingWallet::derive_electrum_addresses(is_testnet, seed_phrase,
                                              num_rcv_addresses, num_chg_addresses,
                                              addresses, addresses_to_data);

        WalletState wallet_state(addresses, addresses_to_data);

        RonghuaClientProvider ronghua_client_provider(electrum_api_client);
        HistoryInspector history_inspector(is_testnet, ronghua_client_provider,
                                           wallet_state);

        history_inspector.create_history_view_rows(true);
        vector<HistoryViewRow> history_view_rows =
                wallet_state.get_history_update();

        cout << "<block>:<amount>:<balance>:<txid>:<p2sh>" << "\n";
        for (auto &r : history_view_rows) {
            cout << r.height << " " << r.balance_delta << " " << r.balance << " "
                 << r.tx_id << " p2sh=" << r.is_p2sh << "\n";
        }

        electrum_api_client.stop();
        return 0;
    }
    catch(exception& e) {
        cerr << e.what() << "\n";
    }
}
