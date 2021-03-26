#include "src/common/bing_common.hpp"
#include <bitcoin/bitcoin.hpp>
#include "src/redeem_script.hpp"
#include <boost/program_options.hpp>

using namespace boost::program_options;
using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;


/**
 * pusher
 * supports single input built from a given funding transaction
 * does not give the rest, funds in excess are treated as fee
 */


void construct_p2sh_time_locking_transaction_from_tx(
        const string src_addr,
        const string priv_key_wif,
        const string src_txid,
        const int src_vout,
        const uint64_t amount_to_transfer,
        const uint32_t lock_until
){
    const wallet::ec_private privKeyEC(priv_key_wif);
    const wallet::ec_public pubKey = privKeyEC.to_public();
    const libbitcoin::config::base16 privKey = libbitcoin::config::base16(privKeyEC.secret());
    data_chunk pubKeyDataChunk;
    pubKey.to_data(pubKeyDataChunk);
    hash_digest srcTxIdDataReversed;
    decode_hash(srcTxIdDataReversed, src_txid);

    // output
    script cltvScript = RedeemScript::to_pay_key_hash_pattern_with_lock(pubKeyDataChunk, lock_until);
    if(!cltvScript.is_valid()){
        std::cout << "CLTV Script Invalid!" << std::endl;
        return;
    }
    short_hash scriptHash = bitcoin_short_hash(cltvScript.to_data(0));
    script pay2ScriptHashLockingScript = script(cltvScript.to_pay_script_hash_pattern(scriptHash));
    output output1(amount_to_transfer, pay2ScriptHashLockingScript);

    // input
    output_point utxo(srcTxIdDataReversed, src_vout);
    input input1 = input();
    input1.set_previous_output(utxo);
    input1.set_sequence(0xfffffffe);

    // tx
    transaction tx = transaction();
    tx.inputs().push_back(input1);
    tx.outputs().push_back(output1);
    tx.set_version(1);

    // sig
    script previousLockingScript = script().to_pay_key_hash_pattern(bitcoin_short_hash(pubKeyDataChunk));
    endorsement sig;
    if(previousLockingScript.create_endorsement(sig, privKeyEC.secret(), previousLockingScript, tx, 0u, all))
    {
        std::cout << "Signature: " << encode_base16(sig) << std::endl;
    }

    // unlocking previous
    operation::list sigScript;
    sigScript.push_back(operation(sig));
    sigScript.push_back(operation(pubKeyDataChunk));
    script scriptUnlockingPreviousLockingScript(sigScript);

    // set unlocking script in input
    tx.inputs()[0].set_script(scriptUnlockingPreviousLockingScript);
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
    /**
     * 1. source address (for reference only)
     * 2. private key for source_addr (note source address as SA)
     * 3. source transaction id (as found out via bx fetch-utxo <satoshis> SA)
     * 4. source transaction's output index (as found out via bx fetch-utxo <satoshis> SA)
     * 5. amount to transfer in Satoshis
     * 6. lock until epoch time (in seconds)
     */
    const string src_addr = "12tohASdGUCDFvqaygaGbL7Jub7CiHdwa4";
    const string priv_key_wif {"L4uNXb2MvLmAtbVMbYg7XsSjgemmyPCnVFaqB4ZX39g8GGNQGqFR"}; // SA = 12tohASdGUCDFvqaygaGbL7Jub7CiHdwa4
    const string src_txid {"22667c482f0f69daefabdf0969be53b8d539e1d2abbfc1c7a193ae38ec0d3e31"}; // bx fetch-utxo 80000 12tohASdGUCDFvqaygaGbL7Jub7CiHdwa4
    const int src_vout {0};
    const uint64_t amount_to_transfer {51000};
    const uint32_t lock_until = 1615381200;

    construct_p2sh_time_locking_transaction_from_tx(src_addr, priv_key_wif, src_txid, src_vout, amount_to_transfer, lock_until);
    return 0;
}

int main(int argc, char* argv[]) {
    try {
        string help_text = "\nYou can find funding transaction by: \n" \
                " 1) bx fetch-balance <funding_address>\n" \
                " 2) if the balance is sufficient, do:\n" \
                "    bx fetch-utxo <desired-amount-in-satoshis> <funding-address>\n" \
                " 3) choose one utxo and capture 'hash' as funding transaction id\n" \
                "    and 'index' as funding transaction output index (vout)\n" \
                "Note that the amount to transfer must be smaller than the available amount in utxo\n" \
                "so that the remainder can be used as a fee.\n" \
                "This program does not give change, you need to use up the entire amount\n" \
                "from the UTXO (divided to funds being locked and to a fee).\n" \
                "For 'lock until' time, use any available online epoch time converter, \n" \
                "note that epoch must be in seconds, not milliseconds. Also note, that the actual\n" \
                "unlocking time will be delayed by around 7 blocks.\n\n" \
                "Private key can be found in your wallet, in Electrum, go to tab 'Addresses',\n" \
                "highlight the desired address, right click and choose `private key'.\n" \
                "Ignore script type part of the key, like 'p2pkh', copy only the key part.\n\n" \
                "This is an offline program, it produces transaction in a hex format that can be broadcast\n" \
                "using any means, for example via 'bx send-tx <tx>' or any online transaction\n" \
                "broadcast drop-off place.\n\n" \
                "Remember that you need to store the unlocking data as printed out by this program,\n" \
                "otherwise your funds will be lost.\n";

        string src_addr;
        string priv_key_wif;
        string src_txid;
        int src_vout;
        uint64_t amount_to_transfer;
        uint32_t lock_until;
        options_description desc("Creates transaction to lock funds via p2sh\n\nRequired options");
        desc.add_options()
            ("help,h", "print usage message")
            ("addr", value<string>(&src_addr)->required(), "funding address")
            ("priv-key,p", value<string>(&priv_key_wif)->required(), "private key to unlock the funding transaction (in WIF format)")
            ("txid,t", value<string>(&src_txid)->required(), "funding transaction id")
            ("vout,v", value<int>(&src_vout)->required(), "funding transaction output index (vout)")
            ("amount", value<uint64_t>(&amount_to_transfer)->required(), "amount to transfer (satoshis)")
            ("lock-until,l", value<uint32_t>(&lock_until)->required(), "lock until epoch time (seconds)")
        ;

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);

        if (vm.count("help") || argc <= 1){
            cout << "\n\n" << desc << "\n";
            cout << "example:" << "\n";
            cout << "--t=d001bd68fc87f05ae3760b4f9c4b64e1000d9194d9c95e0b5a7c7efd933f43d1 --v=0 --amount=890000 --l=1616255893 --p=<private-key> --addr=msWHhBL1vLycmZtQ5M1j7xWuUYvienydfq" << "\n";
            cout << help_text << "\n";
            return 1;
        }

        // note: must be after help option check
        notify(vm);

        construct_p2sh_time_locking_transaction_from_tx(src_addr, priv_key_wif, src_txid, src_vout, amount_to_transfer, lock_until);

        return 0;
    }
    catch(exception& e) {
        cerr << e.what() << "\n";
    }
}