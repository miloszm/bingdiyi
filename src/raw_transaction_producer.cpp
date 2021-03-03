#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client.hpp>


using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;

int main() {
    // private key for source_addr
    const string privKeyWIF {"cN9XS1bFNhMmmvXTNudnwZd7zyuRwCk4HmEVy4xbSxtPArC4KcoE"};
    const wallet::ec_private privKeyEC(privKeyWIF);
    const wallet::ec_public pubKey = privKeyEC.to_public();
    const libbitcoin::config::base16 privKey = libbitcoin::config::base16(privKeyEC.secret());
    // amount to transfer in Satoshis
    const uint64_t satoshisToTransfer {80000};
    // fee in Satoshis - as calculated with help of bx fetch-utxo, where we can see
    // if srcAddr has sufficient funds for the main transfer, the change, and the fee
    const uint64_t fee {10000};
    // source address
    const string srcAddr {""};
    // source transaction id (as found out via bx fetch-tx)
    const string srcTxId {"dd91b944e8b743b1cdb8d106161e8d1f1a666597cc16a6291f7fd4baca891508"};
    // source transaction's output index (as found out via bx fetch-tx)
    const int srcTxOutputIndex {1};
    // main target address
    const string targetAddr {"n2JZCSr8MeGuGtvRVjZTqgNNw9pyYW98Pm"};
    // target address for a change
    // const string targetRemainderAddr {""};


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

    //make Input
    input input1 = input();
    input1.set_previous_output(utxo);
    input1.set_sequence(0xffffffff);

    //build TX
    transaction tx = transaction();
    tx.inputs().push_back(input1);
    tx.outputs().push_back(output1);

    //Endorse TX
    endorsement sig;
    if(lockingScript.create_endorsement(sig, privKeyEC.secret(), lockingScript, tx, 0u, machine::sighash_algorithm::all))
    {
        std::cout << "Signature: " << std::endl;
        std::cout << encode_base16(sig) << "\n" << std::endl;
    }


}
