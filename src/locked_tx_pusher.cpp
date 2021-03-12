#include <bitcoin/bitcoin.hpp>
#include "redeem_script.hpp"


using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;


void construct_p2sh_time_locking_transaction(
        const string privKeyWIF,
        const string srcTxId,
        const int srcTxOutputIndex,
        const uint64_t satoshisToTransfer,
        const uint32_t lockUntil
){
    const wallet::ec_private privKeyEC(privKeyWIF);
    const wallet::ec_public pubKey = privKeyEC.to_public();
    const libbitcoin::config::base16 privKey = libbitcoin::config::base16(privKeyEC.secret());
    data_chunk pubKeyDataChunk;
    pubKey.to_data(pubKeyDataChunk);
    hash_digest srcTxIdDataReversed;
    decode_hash(srcTxIdDataReversed, srcTxId);

    cout << "priv WIF: " << privKeyEC << endl;
    cout << "public hex: " << pubKey << endl;
    cout << "private hex: " << privKey << endl;

    // output
    script cltvScript = RedeemScript::to_pay_key_hash_pattern_with_lock(pubKeyDataChunk, lockUntil);
    if(cltvScript.is_valid())
    {
        std::cout << "CLTV Script is Valid!" << std::endl;
    }else{
        std::cout << "CLTV Script Invalid!" << std::endl;
    }
    short_hash scriptHash = bitcoin_short_hash(cltvScript.to_data(0));
    script pay2ScriptHashLockingScript = script(cltvScript.to_pay_script_hash_pattern(scriptHash));
    output output1(satoshisToTransfer, pay2ScriptHashLockingScript);

    // input
    output_point utxo(srcTxIdDataReversed, srcTxOutputIndex);
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
    std::cout << "Raw Transaction with frozen output until " << lockUntil << ":" << std::endl;
    std::cout << encode_base16(tx.to_data()) << std::endl;
}

int main() {
    const string version {"0.001"};
    cout << "locked_tx_pusher" << "\n";
    cout << "version:" << version << "\n";
    /**
     * 1. private key for source_addr (note source address as SA)
     * 2. source transaction id (as found out via bx fetch-utxo <satoshis> SA)
     * 3. source transaction's output index (as found out via bx fetch-utxo <satoshis> SA)
     * 4. main target address
     * 5. amount to transfer in Satoshis
     * 6. lock until epoch time (in seconds)
     */
    const string privKeyWIF {"L4uNXb2MvLmAtbVMbYg7XsSjgemmyPCnVFaqB4ZX39g8GGNQGqFR"}; // SA = 12tohASdGUCDFvqaygaGbL7Jub7CiHdwa4
    const string srcTxId {"22667c482f0f69daefabdf0969be53b8d539e1d2abbfc1c7a193ae38ec0d3e31"}; // bx fetch-utxo 80000 12tohASdGUCDFvqaygaGbL7Jub7CiHdwa4
    const int srcTxOutputIndex {0};
    const uint64_t satoshisToTransfer {51000};
    const uint32_t lockUntil = 1615381200;

    construct_p2sh_time_locking_transaction(privKeyWIF, srcTxId, srcTxOutputIndex, satoshisToTransfer, lockUntil);
}
