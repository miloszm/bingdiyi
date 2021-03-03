#include <bitcoin/bitcoin.hpp>

using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;

void construct_raw_transaction(
        const string privKeyWIF,
        const string srcTxId,
        const int srcTxOutputIndex,
        const string targetAddr,
        const uint64_t satoshisToTransfer
        ){
    const wallet::ec_private privKeyEC(privKeyWIF);
    const wallet::ec_public pubKey = privKeyEC.to_public();
    const libbitcoin::config::base16 privKey = libbitcoin::config::base16(privKeyEC.secret());

    cout << "priv WIF: " << privKeyEC << endl;
    cout << "public hex: " << pubKey << endl;
    cout << "private hex: " << privKey << endl;

    // make output
    script outputScript = script().to_pay_key_hash_pattern(payment_address(targetAddr).hash());
    output output1(satoshisToTransfer, outputScript);

    // utxo
    string hashString = srcTxId;
    hash_digest utxoHash;
    decode_hash(utxoHash, hashString);
    output_point utxo(utxoHash, srcTxOutputIndex);

    // previous locking script
    data_chunk pubKeyChunk;
    pubKey.to_data(pubKeyChunk);
    script lockingScript = script().to_pay_key_hash_pattern(bitcoin_short_hash(pubKeyChunk));
    std::cout << "\nPrevious Locking Script: " << lockingScript.to_string(0xffffffff) << std::endl;

    // make Input
    input input1 = input();
    input1.set_previous_output(utxo);
    input1.set_sequence(0xffffffff);

    // build TX
    transaction tx = transaction();
    tx.inputs().push_back(input1);
    tx.outputs().push_back(output1);

    // endorse TX
    endorsement sig;
    if(lockingScript.create_endorsement(sig, privKeyEC.secret(), lockingScript, tx, 0u, all))
    {
        std::cout << "Signature: " << std::endl;
        std::cout << encode_base16(sig) << "\n" << std::endl;
    }

    // make Sig Script
    operation::list sigScript;
    sigScript.push_back(operation(sig));
    sigScript.push_back(operation(pubKeyChunk));
    script unlockingScript(sigScript);
    std::cout << "\nUnlocking Script: " << unlockingScript.to_string(0xffffffff) << std::endl;

    // make Signed TX
    tx.inputs()[0].set_script(unlockingScript);
    std::cout << "Raw Transaction: " << std::endl;
    std::cout << encode_base16(tx.to_data()) << std::endl;

}

//private key WIF: cT6gppgsgc84CFxL7mtZHNncHjysHvFrsNoCXa2PHFqfECX2xAeX
//        funding tx id: eeeefc8137dc5c1254f578b027d446a22a119a4473da8867d181a4a232404511
//        funding index: 1
//target address: mihBbdmqPut61bs9eDYZ3fdYxAKEP3mdiX
//        amount: 81000

int main() {
    /**
     *
     * 1. private key for source_addr
     * 2. source transaction id (as found out via bx fetch-tx)
     * 3. source transaction's output index (as found out via bx fetch-tx)
     * 4. main target address
     * 5. amount to transfer in Satoshis
     */
    const string privKeyWIF {"cT6gppgsgc84CFxL7mtZHNncHjysHvFrsNoCXa2PHFqfECX2xAeX"};
    const string srcTxId {"eeeefc8137dc5c1254f578b027d446a22a119a4473da8867d181a4a232404511"};
    const int srcTxOutputIndex {1};
    const string targetAddr {"mihBbdmqPut61bs9eDYZ3fdYxAKEP3mdiX"};
    const uint64_t satoshisToTransfer {81000};

    construct_raw_transaction(privKeyWIF, srcTxId, srcTxOutputIndex, targetAddr, satoshisToTransfer);
}
