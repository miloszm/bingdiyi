#include "../bing_common.hpp"
#include <bitcoin/bitcoin.hpp>
#include "../bing_client.hpp"
#include "../redeem_script.hpp"

using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;


/**
 * autopusher
 *
 * has the ability to extract a single funding transaction for a given address
 * supports only one input so first funding transaction has to have multiple funds
 * this is a transient version - superceeded by autopusher2
 * finds in excess to "satoshisToTransfer" and "fee" are given back as rest
 * so 2 outputs are supported if rest (refund) is needed
 *
 */


void construct_p2sh_time_locking_transaction(
        const string srcAddr,
        const string privKeyWIF,
        const uint64_t satoshisToTransfer,
        const uint64_t satoshisFee,
        const uint32_t lockUntil
){
    BingClient bing_client;
    bing_client.init();

    const wallet::ec_private privKeyEC(privKeyWIF);
    const wallet::ec_public pubKey = privKeyEC.to_public();
    const libbitcoin::config::base16 privKey = libbitcoin::config::base16(privKeyEC.secret());
    data_chunk pubKeyDataChunk;
    pubKey.to_data(pubKeyDataChunk);
//    hash_digest srcTxIdDataReversed;
//    decode_hash(srcTxIdDataReversed, srcTxId);

    cout << "priv WIF: " << privKeyEC << endl;
    cout << "public hex: " << pubKey << endl;
    cout << "private hex: " << privKey << endl;

    cout << "fetch height: " << bing_client.fetchHeight() << "\n";

    auto p = bing_client.fetch_utxo(payment_address(srcAddr), satoshisToTransfer, wallet::select_outputs::algorithm::greedy);
    if (p.points.empty()){
        cout << "Insufficient funds, funding tx not found for " << satoshisToTransfer << " Satoshis" << "\n";
        return;
    }
    auto point {p.points.at(0)};
    auto satoshisAvailable = point.value();
    auto srcTxIdDataReversed = point.hash();
    auto srcTxOutputIndex = point.index();
    if ((satoshisToTransfer + satoshisFee) > satoshisAvailable){
        cout << "Insufficient funds, required " << (satoshisToTransfer + satoshisFee) << ", available " << satoshisAvailable << "\n";
        return;
    }
    auto satoshisRefund = satoshisAvailable - (satoshisToTransfer + satoshisFee);
    cout << "available funds: " << satoshisAvailable << "\n";
    cout << "requested funds: " << satoshisToTransfer << "\n";
    cout << "fee: " << satoshisFee << "\n";
    cout << "refund: " << satoshisRefund << "\n";


    // output 0
    script cltvScript = RedeemScript::to_pay_key_hash_pattern_with_lock(pubKeyDataChunk, lockUntil);
    if(cltvScript.is_valid())
    {
        std::cout << "CLTV Script is Valid!" << std::endl;
    }else{
        std::cout << "CLTV Script Invalid!" << std::endl;
    }
    short_hash scriptHash = bitcoin_short_hash(cltvScript.to_data(0));
    script pay2ScriptHashLockingScript = script(cltvScript.to_pay_script_hash_pattern(scriptHash));
    output output0(satoshisToTransfer, pay2ScriptHashLockingScript);

    // input
    output_point utxo(srcTxIdDataReversed, srcTxOutputIndex);
    input input1 = input();
    input1.set_previous_output(utxo);
    input1.set_sequence(0xfffffffe);

    // tx
    transaction tx = transaction();
    tx.inputs().push_back(input1);
    tx.outputs().push_back(output0);
    if (satoshisRefund > 0){
        output output1(satoshisRefund, script().to_pay_key_hash_pattern(payment_address(srcAddr).hash()));
        tx.outputs().push_back(output1);
    }
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

    const string srcAddr {"mkP2QQqQYsReSpt3JBoRQ5zVdw3ra1jenh"};
    const string privKeyWIF {"cQZ57Q5w1F9YS5n1h81QqnrN2Ea54BMNPCnzoqqgPMdB9wbzwxM6"};
    const uint64_t satoshisToTransfer {500000};
    const uint64_t satoshisFee {10000};
    const uint32_t lockUntil = 1615500000;

    construct_p2sh_time_locking_transaction(srcAddr, privKeyWIF, satoshisToTransfer, satoshisFee, lockUntil);
}