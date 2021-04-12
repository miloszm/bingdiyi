#include "src/common/bing_common.hpp"
#include "src/config/bing_config.hpp"
#include <bitcoin/bitcoin.hpp>
#include <boost/program_options.hpp>
#include <binglib/online_lock_tx_creator.hpp>
#include <binglib/libb_client.hpp>
#include <binglib/purse_accessor.hpp>


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

    vector<string> addresses {"mihBbdmqPut61bs9eDYZ3fdYxAKEP3mdiX"};
    vector<HistoryItem> history_items;

    PurseAccessor::find_history(electrum_api_client, libb_client, addresses, history_items);

    for (HistoryItem& item: history_items){
        cout << item.txid << " " << item.address << "\n";
        if (!item.input.tx.empty())
            cout << "\t\t" << item.input.tx << " " << item.input.output_index << " " << item.input.value << "\n";
        else
            cout << "\t\t\t\t\t\t\t\t" << item.output.script << " " << item.output.value << "\n";
    }

    return 0;
}
