#include "src/common/bing_common.hpp"
#include <chrono>
#include <bitcoin/bitcoin.hpp>
#include "src/libbfunds/purse_accessor.hpp"
#include <boost/program_options.hpp>
#include "src/locktx/online_lock_tx_creator.hpp"
#include <binglib/bing_wallet.hpp>
#include "src/config/bing_config.hpp"
#include <binglib/electrum_api_client.hpp>
#include <binglib/libb_client.hpp>


using namespace boost::program_options;
using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;
using namespace std::chrono;


void create_time_locking_transaction_from_seed(const uint64_t satoshis_to_transfer, const uint64_t satoshis_fee, const uint32_t lock_until, const string seed_phrase) {
    bool is_testnet = true;
    int  num_addresses = 100;

    uint64_t required_funds{satoshis_to_transfer + satoshis_fee};
    LibbClient libb_client;
    libb_client.init(BingConfig::libbitcoin_server_url);
    ElectrumClient electrum_client;
    electrum_client.init(BingConfig::electrum_server_host, BingConfig::electrum_server_service, BingConfig::electrum_cert_file_path);
    ElectrumApiClient electrum_api_client(electrum_client);

    vector<string> addresses;
    map<string, ec_private> addresses_to_ec_private;

    cout << "from m/0/0 to m/0/99: " << "\n";
    BingWallet::derive_addresses(is_testnet, seed_phrase, num_addresses, addresses, addresses_to_ec_private);

    cout << "required funds: " << required_funds << "\n";

    milliseconds ms_before = duration_cast< milliseconds >(system_clock::now().time_since_epoch());

    map<string, uint64_t> address_to_balance;
    AddressFunds funds = PurseAccessor::look_for_funds_by_balance(electrum_api_client, libb_client, required_funds, addresses, address_to_balance);
//    AddressFunds funds = PurseAccessor::look_for_funds(libb_client, required_funds, addresses);
    for (auto a: address_to_balance){
        cout << a.first << " " << a.second << "\n";
    }

    milliseconds ms_after = duration_cast< milliseconds >(system_clock::now().time_since_epoch());

    int ms_elapsed = ms_after.count() - ms_before.count();

    cout << "took " << ms_elapsed << " ms\n";

    if (funds.actual_funds >= funds.requested_funds) {
        cout << "funds found:" << "\n";
        cout << "address = " << funds.address << "\n";
        cout << "requested funds = " << funds.requested_funds << "\n";
        cout << "actual funds = " << funds.actual_funds << "\n";
        cout << "refund = " << funds.actual_funds - funds.requested_funds << "\n";
        cout << "number of inputs = " << funds.points.size() << "\n";
    }

    string source_address = funds.address;
    ec_private private_key = addresses_to_ec_private[funds.address];

    OnlineLockTxCreator::construct_p2sh_time_locking_transaction_from_address(source_address, private_key, satoshis_to_transfer, satoshis_fee, lock_until);
}

int main(int argc, char* argv[]){
    try {
        string help_text = "\nYou only provide Electrum mnemonic seed phrase and the program will\n" \
                "find the funding transaction automatically.\n\n" \
                "Note that all funds must be under a single address, multiple addresses will not\n" \
                "be gathered to contribute their funds to the desired amount.\n\n" \
                "This program does give change, if any, it will be transferred back into the source address.\n" \
                "For 'lock until' time, use any available online epoch time converter, \n" \
                "note that epoch must be in seconds, not milliseconds. Also note, that the actual\n" \
                "unlocking time will be delayed by around 7 blocks.\n\n" \
                "This program produces transaction in a hex format that can be broadcast\n" \
                "using any means, for example via 'bx send-tx <tx>' or any online transaction\n" \
                "broadcast drop-off place.\n\n" \
                "Remember that you need to store the unlocking data as printed out by this program,\n" \
                "otherwise your funds will be lost.\n";

        uint64_t amount_to_transfer {0};
        uint64_t fee {0};
        uint32_t lock_until {0};
        string seed_phrase {"effort canal zoo clown shoulder genuine penalty moral unit skate few quick"};

        options_description desc("Creates transaction to lock funds via p2sh\n\nRequired options");
        desc.add_options()
                ("help,h", "print usage message")
                ("seed,s", value<string>(&seed_phrase)->required(), "Electrum seed phrase")
                ("amount", value<uint64_t>(&amount_to_transfer)->required(), "amount to transfer (satoshis)")
                ("fee,f", value<uint64_t>(&fee)->required(), "fee (satoshis), note: amount+fee <= available funds")
                ("lock-until,l", value<uint32_t>(&lock_until)->required(), "lock until epoch time (seconds)")
                ;

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);

        if (vm.count("help") || argc <= 1){
            cout << "\n\n" << desc << "\n";
            cout << "example:" << "\n";
            cout << "--amount=890000 --fee=5000 --l=1616255000 --s=""\"effort canal zoo clown shoulder genuine penalty moral unit skate few quick\"" << "\n";
            cout << help_text << "\n";
            return 1;
        }

        // note: must be after help option check
        notify(vm);

        create_time_locking_transaction_from_seed(amount_to_transfer, fee, lock_until, seed_phrase);
        return 0;
    }
    catch(exception& e) {
        cerr << e.what() << "\n";
    }
}
