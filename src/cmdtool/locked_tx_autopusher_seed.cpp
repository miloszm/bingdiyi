#include "src/common/bing_common.hpp"
#include <bitcoin/bitcoin.hpp>
#include "src/bing_client.hpp"
#include "src/purse_accessor.hpp"
#include <boost/program_options.hpp>
#include "src/locktx/online_lock_tx_creator.hpp"

using namespace boost::program_options;
using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;


void create_time_locking_transaction_from_seed(const uint64_t satoshis_to_transfer, const uint64_t satoshis_fee, const uint32_t lock_until, const string seed_phrase, const string src_addr_hint) {
    uint64_t required_funds{satoshis_to_transfer + satoshis_fee};
    BingClient bing_client;
    bing_client.init();

    const word_list mnemonic = split(seed_phrase, " ");

    if (electrum::validate_mnemonic(mnemonic, language::en)){
        cout << "mnemonic validated OK" << "\n";
    } else {
        cout << "mnemonic BAD" << "\n";
    }

    long_hash seed = electrum::decode_mnemonic(mnemonic);

    cout << "seed=" << "\n";
    cout << config::base16(seed) << "\n";

    data_chunk seedAsChunk(seed.begin(), seed.end());

    const hd_private m(seedAsChunk, hd_private::testnet);
    const hd_public m_pub = m;

    auto m0_pub = m.derive_public(0);
    auto m1_pub = m.derive_public(1);

    cout << m_pub.encoded() << "\n";
    cout << m0_pub.encoded() << "\n";
    cout << m1_pub.encoded() << "\n";

    hd_private m0 = m.derive_private(0);

    vector<string> addresses;
    map<string, ec_private> addresses_to_ec_private;

    // from m/0/0 to m/0/99
    cout << "from m/0/0 to m/0/99: " << "\n";
    for (int i = 0; i < 100; ++i){
        hd_private hdPrivate = m0.derive_private(i);
        const payment_address address({ hdPrivate.secret(), payment_address::testnet_p2kh });
        cout << "m/0/" << i <<" address: " << address.encoded() << "\n";
        addresses.push_back(address.encoded());
        addresses_to_ec_private[address.encoded()] = ec_private(hdPrivate.secret(), payment_address::testnet_p2kh);
    }

    if (!src_addr_hint.empty()) {
        auto a = find(addresses.begin(), addresses.end(), src_addr_hint);
        if (a != addresses.end()){
            addresses = vector<string>{src_addr_hint};
        } else {
            cout << "address not found: " << src_addr_hint << "\n";
            return;
        }
    }

    cout << "required funds: " << required_funds << "\n";
    AddressFunds funds = PurseAccessor::look_for_funds(bing_client, required_funds, addresses);

    cout << "funds found:" << "\n";
    cout << "address = " << funds.address << "\n";
    cout << "requested funds = " << funds.requested_funds << "\n";
    cout << "actual funds = " << funds.actual_funds << "\n";
    cout << "number of inputs = " << funds.points.size() << "\n";

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

        string   src_addr_hint {""};
        uint64_t amount_to_transfer;
        uint64_t fee;
        uint32_t lock_until;
        string seed_phrase {"effort canal zoo clown shoulder genuine penalty moral unit skate few quick"};

        options_description desc("Creates transaction to lock funds via p2sh\n\nRequired options");
        desc.add_options()
                ("help,h", "print usage message")
                ("seed,s", value<string>(&seed_phrase)->required(), "Electrum seed phrase")
                ("addr", value<string>(&src_addr_hint), "Source address hint")
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

        create_time_locking_transaction_from_seed(amount_to_transfer, fee, lock_until, seed_phrase, src_addr_hint);
        return 0;
    }
    catch(exception& e) {
        cerr << e.what() << "\n";
    }
}
