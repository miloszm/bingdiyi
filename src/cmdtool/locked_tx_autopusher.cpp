#include "src/common/bing_common.hpp"
#include <bitcoin/system.hpp>
#include <boost/program_options.hpp>
#include <binglib/online_lock_tx_creator.hpp>
#include <binglib/libb_client.hpp>


using namespace boost::program_options;
using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;


/**
 * autopusher
 *
 * has the ability to extract funding transactions for a given address
 * support multiple inputs if needed
 * funds in excess to "amount_to_transfer" and "fee" are transferred back to the funding address
 * (2 outputs are generated if the change (refund) is non-zero)
 *
 */




int main(int argc, char* argv[]) {
    try {
        string help_text = "\nYou can find funding address by inspecting your wallet.\n" \
                "Note that the amount to transfer plus fee must be smaller than or equal to the available amount for a given address.\n" \
                "This program does give change, if any, it will be transferred back into the source address.\n" \
                "For 'lock until' time, use any available online epoch time converter, \n" \
                "note that epoch must be in seconds, not milliseconds. Also note, that the actual\n" \
                "unlocking time will be delayed by around 7 blocks.\n" \
                "Private key can be found in your wallet, in Electrum, go to tab 'Addresses',\n" \
                "highlight the desired address, right click and choose `private key'.\n" \
                "Ignore script type part of the key, like 'p2pkh', copy only the key part.\n\n" \
                "This program produces transaction in a hex format that can be broadcast\n" \
                "using any means, for example via 'bx send-tx <tx>' or any online transaction\n" \
                "broadcast drop-off place.\n\n" \
                "Remember that you need to store the unlocking data as printed out by this program,\n" \
                "otherwise your funds will be lost.\n";

        string src_addr;
        string priv_key_wif;
        uint64_t amount_to_transfer;
        uint64_t fee;
        uint32_t lock_until;
        bool is_testnet {true};
        options_description desc("Creates transaction to lock funds via p2sh\n\nRequired options");
        desc.add_options()
                ("help,h", "print usage message")
                ("addr", value<string>(&src_addr)->required(), "funding address")
                ("priv-key,p", value<string>(&priv_key_wif)->required(), "private key for the funding address (in WIF format)")
                ("amount", value<uint64_t>(&amount_to_transfer)->required(), "amount to transfer (satoshis)")
                ("fee,f", value<uint64_t>(&fee)->required(), "fee (satoshis), note: amount+fee <= available funds")
                ("lock-until,l", value<uint32_t>(&lock_until)->required(), "lock until epoch time (seconds)")
                ("testnet,t", value<bool>(&is_testnet)->default_value(true),"use testnet blockchain");

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);

        if (vm.count("help") || argc <= 1){
            cout << "\n\n" << desc << "\n";
            cout << "example:" << "\n";
            cout << "--amount=890000 --fee=5000 --l=1616255893 --p=<private-key> --addr=msWHhBL1vLycmZtQ5M1j7xWuUYvienydfq" << "\n";
            cout << help_text << "\n";
            return 1;
        }

        // note: must be after help option check
        notify(vm);

        std::ifstream ifs("config.json");
        json config_json = json::parse(ifs);
        string blockchain = is_testnet ? "testnet" : "mainnet";
        string libbitcoin_server_url = config_json[blockchain]["libbitcoin_connection"]["url"];

        LibbClient libb_client;
        libb_client.init(libbitcoin_server_url);

        LockTxInfo lock_tx_info =
            OnlineLockTxCreator::construct_p2sh_time_locking_tx_from_address(
                libb_client, src_addr, priv_key_wif, amount_to_transfer, fee,
                lock_until);
        cout << lock_tx_info.unlocking_info << "\n";
        cout << lock_tx_info.locking_tx << "\n";

        return 0;
    }
    catch(exception& e) {
        cerr << e.what() << "\n";
    }
}
