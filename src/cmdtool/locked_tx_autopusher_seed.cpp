#include "src/common/bing_common.hpp"
#include <chrono>
#include <bitcoin/system.hpp>
#include <binglib/purse_accessor.hpp>
#include <boost/program_options.hpp>
#include <binglib/online_lock_tx_creator.hpp>
#include <binglib/bing_wallet.hpp>
#include "src/config/bing_config.hpp"
#include <binglib/ronghua_client.hpp>
#include <binglib/libb_client.hpp>

using namespace boost::program_options;
using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;
using namespace std::chrono;

void create_time_locking_transaction_from_seed(
    LibbClient &libb_client, RonghuaClient &electrum_api_client,
    const uint64_t satoshis_to_transfer, const uint64_t satoshis_fee,
    const uint32_t lock_until, const string seed_phrase,
    int num_rcv_addresses, int num_chg_addresses) {
    try {
        bool is_testnet = true;
        uint64_t required_funds{satoshis_to_transfer + satoshis_fee};

        vector<string> addresses;
        map<string, AddressDerivationResult> addresses_to_data;

        cout << "deriving addresses:" << " m/0/0 .. m/0/" << num_rcv_addresses-1 << ", m/1/0 .. m/1/" << num_chg_addresses-1 << "\n";
        BingWallet::derive_electrum_addresses(is_testnet, seed_phrase,
                                              num_rcv_addresses, num_chg_addresses,
                                              addresses, addresses_to_data);

        cout << "required funds: " << required_funds << "\n";

        milliseconds ms_before =
            duration_cast<milliseconds>(system_clock::now().time_since_epoch());

        map<string, uint64_t> address_to_balance;
        AddressFunds funds = PurseAccessor::look_for_funds_by_balance(
            electrum_api_client, libb_client, required_funds, addresses,
            address_to_balance);

        milliseconds ms_after =
            duration_cast<milliseconds>(system_clock::now().time_since_epoch());

        int ms_elapsed = ms_after.count() - ms_before.count();

        cout << "took " << ms_elapsed << " ms\n";

        cout << "\n";
        if (funds.actual_funds >= funds.requested_funds) {
            cout << "funds found:"
                 << "\n";
            cout << "address = " << funds.address << "\n";
            cout << "requested funds = " << funds.requested_funds << "\n";
            cout << "actual funds = " << funds.actual_funds << "\n";
            cout << "refund = " << funds.actual_funds - funds.requested_funds
                 << "\n";
            cout << "number of inputs = " << funds.points.size() << "\n";
        }
        cout << "\n";

        string source_address = funds.address;
        ec_private private_key = addresses_to_data[funds.address].priv_key_ec;

        LockTxInfo lock_tx_info =
            OnlineLockTxCreator::construct_p2sh_time_locking_tx_from_address(
                electrum_api_client, source_address, private_key,
                satoshis_to_transfer, satoshis_fee, lock_until);
        cout << lock_tx_info.unlocking_info << "\n";
        cout << lock_tx_info.locking_tx << "\n";
    } catch (exception &e) {
        cerr << "exception in create_time_locking_transaction_from_seed: "
             << e.what() << "\n";
    }
}

int main(int argc, char *argv[]) {
    const int DEFAULT_NUM_RCV_ADDRESSES {50};
    const int DEFAULT_NUM_CHG_ADDRESSES {50};
    const int MIN_NUM_ADDRESSES {10};
    const int MAX_NUM_ADDRESSES {10000};

    try {
        string help_text =
            "\nYou only provide Electrum mnemonic seed phrase and the program will\n"
            "find the funding transaction automatically.\n\n"
            "Note that all funds must be under a single address, multiple addresses will not\n"
            "be gathered to contribute their funds to the desired amount.\n\n"
            "This program does give change, if any, it will be transferred back into the source address.\n"
            "For 'lock until' time, use any available online epoch time converter, \n"
            "note that epoch must be in seconds, not milliseconds. Also note, that the actual\n"
            "unlocking time will be delayed by around 7 blocks.\n\n"
            "This program produces transaction in a hex format that can be broadcast\n"
            "using any means, for example via 'bx send-tx <tx>' or any online transaction\n"
            "broadcast drop-off place.\n\n"
            "Remember that you need to store the unlocking data as printed out by this program,\n"
            "otherwise your funds will be lost.\n";

        int num_rcv_addresses {DEFAULT_NUM_RCV_ADDRESSES};
        int num_chg_addresses {DEFAULT_NUM_CHG_ADDRESSES};

        uint64_t amount_to_transfer{0};
        uint64_t fee{0};
        uint32_t lock_until{0};
        string seed_phrase{""};

        options_description desc(
            "Creates transaction to lock funds via p2sh\n\nRequired options");
        desc.add_options()
            ("help,h", "print usage message")
            ("seed,s", value<string>(&seed_phrase)->required(), "Electrum seed phrase")
            ("amount", value<uint64_t>(&amount_to_transfer)->required(), "amount to transfer (satoshis)")
            ("fee,f", value<uint64_t>(&fee)->required(), "fee (satoshis), note: amount+fee <= available funds")
            ("lock-until,l", value<uint32_t>(&lock_until)->required(), "lock until epoch time (seconds)")
            ("receiving addresses,r", value<int>(&num_rcv_addresses)->default_value(DEFAULT_NUM_RCV_ADDRESSES), "number of receiving addresses")
            ("change addresses,c", value<int>(&num_chg_addresses)->default_value(DEFAULT_NUM_CHG_ADDRESSES),"number of change addresses");

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);

        if (vm.count("help") || argc <= 1) {
            cout << "\n\n" << desc << "\n";
            cout << "example:"
                 << "\n";
            cout
                << "--amount=90000 --fee=5000 --l=1616255000 --s=\"effort canal zoo clown shoulder genuine penalty moral unit skate few quick\""
                << "\n";
            cout << help_text << "\n";
            return 1;
        }

        // note: must be after help option check
        notify(vm);

        if (num_rcv_addresses < MIN_NUM_ADDRESSES || num_rcv_addresses > MAX_NUM_ADDRESSES){
            num_rcv_addresses = DEFAULT_NUM_RCV_ADDRESSES;
        }

        if (num_chg_addresses < MIN_NUM_ADDRESSES || num_chg_addresses > MAX_NUM_ADDRESSES){
            num_chg_addresses = DEFAULT_NUM_CHG_ADDRESSES;
        }

        LibbClient libb_client;
        libb_client.init(BingConfig::libbitcoin_server_url);
        RonghuaClient electrum_api_client;
        electrum_api_client.init(BingConfig::electrum_server_host,
                                 BingConfig::electrum_server_service,
                                 BingConfig::electrum_cert_file_path);

        create_time_locking_transaction_from_seed(
            libb_client, electrum_api_client, amount_to_transfer, fee,
            lock_until, seed_phrase, num_rcv_addresses, num_chg_addresses);

        electrum_api_client.stop();
        return 0;
    } catch (exception &e) {
        cerr << e.what() << "\n";
    }
}
