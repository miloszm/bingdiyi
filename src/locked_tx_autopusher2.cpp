#include "bing_common.hpp"
#include <bitcoin/bitcoin.hpp>
#include "bing_client.hpp"
#include "redeem_script.hpp"
#include "funds_finder.hpp"
#include <boost/program_options.hpp>

using namespace boost::program_options;
using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;


/**
 * autopusher2
 *
 * has the ability to extract funding transactions for a given address
 * support multiple inputs if needed
 * funds in excess to "amount_to_transfer" and "fee" are transferred back to the funding address
 * (2 outputs are generated if the change (refund) is non-zero)
 *
 */



void construct_p2sh_time_locking_transaction_from_address(
        const string src_addr,
        const string priv_key_wif,
        const uint64_t amount_to_transfer,
        const uint64_t satoshis_fee,
        const uint32_t lock_until
){
    BingClient bing_client;
    bing_client.init();

    const wallet::ec_private priv_key_ec(priv_key_wif);
    const wallet::ec_public pub_key = priv_key_ec.to_public();
    const libbitcoin::config::base16 priv_key = libbitcoin::config::base16(priv_key_ec.secret());
    data_chunk pub_key_data_chunk;
    pub_key.to_data(pub_key_data_chunk);

    auto points_value = bing_client.fetch_utxo(payment_address(src_addr), 1, wallet::select_outputs::algorithm::individual);
    auto satoshis_needed = amount_to_transfer + satoshis_fee;
    auto utxos_funds = FundsFinder::find_funds(satoshis_needed, points_value);
    auto utxos = utxos_funds.first;
    auto available_funds = utxos_funds.second;
    if (utxos_funds.first.empty()){
        cout << "Insufficient funds, required " << satoshis_needed << ", available " << available_funds << "\n";
        return;
    }
    auto refund = available_funds - satoshis_needed;
    cout << "available funds: " << available_funds << "\n";
    cout << "requested funds: " << amount_to_transfer << "\n";
    cout << "fee: " << satoshis_fee << "\n";
    cout << "refund: " << refund << "\n";

    // output 0
    script cltvScript = RedeemScript::to_pay_key_hash_pattern_with_lock(pub_key_data_chunk, lock_until);
    if(cltvScript.is_valid())
    {
        std::cout << "CLTV Script is Valid!" << std::endl;
    }else{
        std::cout << "CLTV Script Invalid!" << std::endl;
    }
    short_hash script_hash = bitcoin_short_hash(cltvScript.to_data(0));
    script pay2ScriptHashLockingScript = script(cltvScript.to_pay_script_hash_pattern(script_hash));
    output output0(amount_to_transfer, pay2ScriptHashLockingScript);

    // tx
    transaction tx = transaction();
    for(auto utxo: utxos){
        input input1 = input();
        input1.set_previous_output(utxo);
        input1.set_sequence(0xfffffffe);
        tx.inputs().push_back(input1);
    }
    tx.outputs().push_back(output0);
    if (refund > 0){
        output output1(refund, script().to_pay_key_hash_pattern(payment_address(src_addr).hash()));
        tx.outputs().push_back(output1);
    }
    tx.set_version(1);

    // set unlocking script in inputs
    for (unsigned int i = 0; i < utxos.size(); ++i) {
        // sig
        script previousLockingScript = script().to_pay_key_hash_pattern(bitcoin_short_hash(pub_key_data_chunk));
        endorsement sig;
        if(previousLockingScript.create_endorsement(sig, priv_key_ec.secret(), previousLockingScript, tx, i, all))
        {
            std::cout << "Signature: " << encode_base16(sig) << std::endl;
        }
        // unlocking previous
        operation::list sigScript;
        sigScript.push_back(operation(sig));
        sigScript.push_back(operation(pub_key_data_chunk));
        script scriptUnlockingPreviousLockingScript(sigScript);

        tx.inputs()[i].set_script(scriptUnlockingPreviousLockingScript);
    }

    cout << "==========================" << "\n";
    cout << "==========================" << "\n";
    std::cout << "Transaction with frozen output until " << lock_until << ":" << std::endl;
    std::cout << encode_base16(tx.to_data()) << std::endl;
    cout << "==========================" << "\n";
    cout << "==========================" << "\n";

    string tx_to_unlock = encode_hash(tx.hash());

    cout << "===== data to unlock: ====" << "\n";
    cout << "lock time: " << lock_until << "\n";
    cout << "private key of address: " << src_addr << "\n";
    cout << "available amount: " << amount_to_transfer << "\n";
    cout << "from ^^ please subtract fee" << "\n";
    cout << "funding transaction id to unlock: " << tx_to_unlock << "\n";
    cout << "desired target address where the unlocked funds will be transferred" << "\n";
    cout << "==========================" << "\n";
    cout << "==========================" << "\n";

}

int main2() {
    const string version {"0.001"};
    cout << "locked_tx_pusher" << "\n";
    cout << "version:" << version << "\n";

    const string src_addr {"mkP2QQqQYsReSpt3JBoRQ5zVdw3ra1jenh"};
    const string priv_key_wif {"cQZ57Q5w1F9YS5n1h81QqnrN2Ea54BMNPCnzoqqgPMdB9wbzwxM6"};
    const uint64_t satoshis_to_transfer {2000000};
    const uint64_t satoshis_fee {10000};
    const uint32_t lock_until = 1615591800;

    construct_p2sh_time_locking_transaction_from_address(src_addr, priv_key_wif, satoshis_to_transfer, satoshis_fee, lock_until);
    return 0;
}

int main(int argc, char* argv[]) {
    try {
        string help_text = "\nYou can find funding address by inspecting your wallet.\n" \
                "Note that the amount to transfer plus fee must be smaller than or equal to the available amount for a given addres.\n" \
                "This program does give change, if any, it will be transferred back into the source address.\n" \
                "For 'lock until' time, use any available online epoch time converter, \n" \
                "note that epoch must be in seconds, not milliseconds. Also note, that the actual\n" \
                "unlocking time will be delayed be around 7 blocks.\n" \
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
        options_description desc("Required options");
        desc.add_options()
                ("help,h", "print usage message")
                ("addr", value<string>(&src_addr)->required(), "funding address")
                ("priv-key,p", value<string>(&priv_key_wif)->required(), "private key for the funding address (in WIF format)")
                ("amount", value<uint64_t>(&amount_to_transfer)->required(), "amount to transfer (satoshis)")
                ("fee,f", value<uint64_t>(&fee)->required(), "fee (satoshis), note: amount+fee <= available funds")
                ("lock-until,l", value<uint32_t>(&lock_until)->required(), "lock until epoch time (seconds)")
                ;

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

        construct_p2sh_time_locking_transaction_from_address(src_addr, priv_key_wif, amount_to_transfer, fee, lock_until);

        return 0;
    }
    catch(exception& e) {
        cerr << e.what() << "\n";
    }
}
