#include <bitcoin/system.hpp>
#include "redeem_script.hpp"

using namespace std;
using namespace bc;
using namespace bc::system::chain;
using namespace bc::system::wallet;
using namespace bc::system::machine;

/**
 * spender
 *
 * spends (unlocks) funds locked via p2sh script containing OP_CLTV
 *
 */


void construct_raw_transaction(
        const string privKeyWIF,
        const string srcTxId,
        const int srcTxOutputIndex,
        const uint32_t srcLockUntil,
        const string targetAddr,
        const uint64_t satoshisToTransfer
){
    const wallet::ec_private privKeyEC(privKeyWIF);
    const wallet::ec_public pubKey = privKeyEC.to_public();
    const libbitcoin::config::base16 privKey = libbitcoin::config::base16(privKeyEC.secret());
    data_chunk pubKeyChunk;
    pubKey.to_data(pubKeyChunk);

    cout << "priv WIF: " << privKeyEC << endl;
    cout << "public hex: " << pubKey << endl;
    cout << "private hex: " << privKey << endl;

    // bx tx-encode -i 2a0990b736e79e1d65ce3e9e25427e36855235829d58c1f2a9eac18142c926a6:0:0 -o n4eaAFB3GPmrJR4ummYpQmYTx2VaNftuPe:800000 -l 1615161540

    string hashString = srcTxId;
    hash_digest utxoHash;
    decode_hash(utxoHash, hashString);
    output_point utxo(utxoHash, srcTxOutputIndex);
    input input1 = input();
    input1.set_previous_output(utxo);
    input1.set_sequence(0);

    script currentLockingScript = script().to_pay_key_hash_pattern(payment_address(targetAddr).hash());
    output output1(satoshisToTransfer, currentLockingScript);

    transaction tx = transaction();
    tx.inputs().push_back(input1);
    tx.outputs().push_back(output1);
    tx.set_locktime(srcLockUntil);
    tx.set_version(1);


    //bx input-sign fd2fc82cc442f35b3b577dc8f300d80007cc53c8d3f922265ccdc84e5c2729d5 "[50714760] checklocktimeverify drop [0375253f2f96889d04eda186cfd2f0f161f4888e538066acb39adb1729ed374e4e] checksig" 010000000155eb2941e57ebf58b0296f114bad51c459e72df3308964ff9c95803fe91c49a80000000000000000000130570500000000001976a9147bc59a29fdd04f10d03ae5f3668a36163ffc580688ac50714760

    cout << "input to sig tx " << encode_base16(tx.to_data()) << "\n";
    cout << "should          " << "010000000155eb2941e57ebf58b0296f114bad51c459e72df3308964ff9c95803fe91c49a80000000000000000000130570500000000001976a9147bc59a29fdd04f10d03ae5f3668a36163ffc580688ac50714760" << "\n";

    script redeemScript = RedeemScript::to_pay_key_hash_pattern_with_lock(pubKeyChunk, srcLockUntil);
    if(redeemScript.is_valid())
    {
        std::cout << "CLTV Script is Valid!" << std::endl;
    }else{
        std::cout << "CLTV Script Invalid!" << std::endl;
    }
    endorsement sig;
    if(script::create_endorsement(sig, privKeyEC.secret(), redeemScript, tx, 0u, all))
    {
        std::cout << "Signature: " << encode_base16(sig) << std::endl;
    }

    // bx input-set "zero [3044022024048cd26f0d493173c4c1e15be7fc4bb0c9f91bbba422d46d09b910ec28c0ac02202477ce166b20b13aee4e997c5e2ffbdd8d274d06066307690f75dd7dcc5a3a6a01] [04c4684560b175210314488ebfec9889c4253fe2d1a21715b932864d2892193e4ca60e0acbd1d9fd1dac]" 0100000001a626c94281c1eaa9f2c1589d82355285367e42259e3ece651d9ee736b790092a0000000000000000000100350c00000000001976a914fdbbbe6062fef2fca812e404e3dcb43dcdb4108888acc4684560

    operation::list sigScript;
    sigScript.push_back(operation(sig));
    sigScript.push_back(redeemScript.to_data(0));
    script scriptUnlockingPreviousLockingScript(sigScript);
    tx.inputs()[0].set_script(scriptUnlockingPreviousLockingScript);

    std::cout << "Raw Transaction: " << std::endl;
    std::cout << encode_base16(tx.to_data()) << std::endl;
}

int main() {
    const string version {"0.001"};
    cout << "locked_tx_spender" << "\n";
    cout << "version:" << version << "\n";

    const string privKeyWIF {"cQZ57Q5w1F9YS5n1h81QqnrN2Ea54BMNPCnzoqqgPMdB9wbzwxM6"};
    const string srcTxId {"471a1fe1e09fe1ba5fdc0ea9476607a3b5871146e34c0b5650f8a618cbb60aa4"};
    const int srcTxOutputIndex {0};
    const uint64_t satoshisToTransfer {1950000};
    const uint32_t srcLockUntil = 1615591800;
    const string targetAddr {"mpS14bFCZiHFRxfNNbnPT2FScJBrm96iLE"};

    construct_raw_transaction(privKeyWIF, srcTxId, srcTxOutputIndex, srcLockUntil, targetAddr, satoshisToTransfer);
}
