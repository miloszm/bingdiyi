#include "src/common/bing_common.hpp"
#include "src/config/bing_config.hpp"
#include <bitcoin/system.hpp>
#include <boost/program_options.hpp>
#include <binglib/online_p2pkh_tx_creator.hpp>
#include <binglib/libb_client.hpp>


using namespace boost::program_options;
using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;

/**
 * produces p2pkh transaction
 * no locking, no p2sh, just plan and simple p2pkh
 * gives rest
 * multiple inputs, 1 or 2 outputs (2 is the rest is non-zero)
 * rest is paid back to source address
 * only single funding source address is supported
 */




int main(int argc, char* argv[]) {
    try {
        LibbClient libb_client;
        libb_client.init(BingConfig::libbitcoin_server_url);

        string help_text = "\nYou can find funding address by inspecting your wallet.\n" \
                "Note that the amount to transfer plus fee must be smaller than or equal to the available amount for a given address.\n" \
                "This program does give change, if any, it will be transferred back into the source address.\n" \
                "Private key can be found in your wallet, in Electrum, go to tab 'Addresses',\n" \
                "highlight the desired address, right click and choose `private key'.\n" \
                "Ignore script type part of the key, like 'p2pkh', copy only the key part.\n\n" \
                "This program produces transaction in a hex format that can be broadcast\n" \
                "using any means, for example via 'bx send-tx <tx>' or any online transaction\n" \
                "broadcast drop-off place.\n\n";

        string src_addr;
        string priv_key_wif;
        uint64_t amount_to_transfer;
        uint64_t fee;
        string target_addr;
        options_description desc("Creates transaction to transfer funds via p2pkh\n\nRequired options");
        desc.add_options()
                ("help,h", "print usage message")
                ("addr", value<string>(&src_addr)->required(), "funding address")
                ("priv-key,p", value<string>(&priv_key_wif)->required(), "private key for the funding address (in WIF format)")
                ("amount", value<uint64_t>(&amount_to_transfer)->required(), "amount to transfer (satoshis)")
                ("fee,f", value<uint64_t>(&fee)->required(), "fee (satoshis), note: amount+fee <= available funds")
                ("target,t", value<string>(&target_addr)->required(), "target address")
                ;

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);

        if (vm.count("help") || argc <= 1){
            cout << "\n\n" << desc << "\n";
            cout << "example:" << "\n";
            cout << "--amount=90000 --fee=5000 --p=<private-key> --addr=msWHhBL1vLycmZtQ5M1j7xWuUYvienydfq --target=morHRfjX3sQ4R2BRSyjij8yePAW2XKHWd3" << "\n";
            cout << help_text << "\n";
            return 1;
        }

        // note: must be after help option check
        notify(vm);

        string tx_hex = OnlineP2pkhTxCreator::construct_p2pkh_tx_from_address(libb_client, src_addr, priv_key_wif, amount_to_transfer, fee, target_addr);

        cout << "==========================" << "\n";
        cout << "==========================" << "\n";
        std::cout << "Transaction:" << std::endl;
        std::cout << tx_hex << std::endl;
        cout << "==========================" << "\n";
        cout << "==========================" << "\n";

        return 0;
    }
    catch(exception& e) {
        cerr << e.what() << "\n";
    }
}