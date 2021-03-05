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

    /**
     * make output
     * payment_address decodes base58 address and calculates hash for it
     * to_pay_key_hash_pattern then creates the script, filling out the address
     */
    script currentLockingScript = script().to_pay_key_hash_pattern(payment_address(targetAddr).hash());
    output output1(satoshisToTransfer, currentLockingScript);

    /**
     * make utxo
     * funding transaction id is decoded from hex string into bytes and reversed
     * (hash_digest is a plain array of 32 bytes)
     */
    string hashString = srcTxId;
    hash_digest utxoHash;
    decode_hash(utxoHash, hashString);
    output_point utxo(utxoHash, srcTxOutputIndex);

    /**
     * previous locking script
     * convert public key to chunk data, and then calculate sha 256 and ripemd160 on it
     * then place it in a locking script created from scratch
     * previous locking script is not read, but rather recreated from scratch here
     * it is needed for signing only, otherwise it is not used
     */
    data_chunk pubKeyChunk;
    pubKey.to_data(pubKeyChunk);
    script previousLockingScript = script().to_pay_key_hash_pattern(bitcoin_short_hash(pubKeyChunk));
    std::cout << "\nPrevious Locking Script: " << previousLockingScript.to_string(0xffffffff) << std::endl;

    /**
     * make input
     * input consists of previous output, which is utxo
     */
    input input1 = input();
    input1.set_previous_output(utxo);
    input1.set_sequence(0xffffffff);

    /**
     * build TX
     * we build transaction with inputs and outputs
     * input is not endorsed at this moment yet
     */
    transaction tx = transaction();
    tx.inputs().push_back(input1);
    tx.outputs().push_back(output1);

    /**
     * build endorsement
     * endorsement is created out of the private key, locking script, transaction, input index
     * endorsement is a hashed signature of provided data
     */
    endorsement sig;
    if(previousLockingScript.create_endorsement(sig, privKeyEC.secret(), previousLockingScript, tx, 0u, all))
    {
        std::cout << "Signature: " << encode_base16(sig) << std::endl;
    }

    /**
     * make Sig Script
     * unlocking script is created from scratch here as a list of operations
     */
    operation::list sigScript;
    sigScript.push_back(operation(sig));
    sigScript.push_back(operation(pubKeyChunk));
    script scriptUnlockingPreviousLockingScript(sigScript);
    std::cout << "\nUnlocking Script: " << scriptUnlockingPreviousLockingScript.to_string(0xffffffff) << std::endl;

    /**
     * make Signed TX
     * fill out input with unlocking script which was missing until this point
     */
    tx.inputs()[0].set_script(scriptUnlockingPreviousLockingScript);
    std::cout << "Raw Transaction: " << std::endl;
    std::cout << encode_base16(tx.to_data()) << std::endl;
}

int main() {
    /**
     * 1. private key for source_addr (note source address as SA)
     * 2. source transaction id (as found out via bx fetch-utxo <satoshis> SA)
     * 3. source transaction's output index (as found out via bx fetch-utxo <satoshis> SA)
     * 4. main target address
     * 5. amount to transfer in Satoshis
     */
    const string privKeyWIF {"cSv9QafnL7UxFDdbRe7G9JtzWn3RoV1GCW9FfFzjDgLUNZgsBwsA"}; // SA = n2JZCSr8MeGuGtvRVjZTqgNNw9pyYW98Pm
    const string srcTxId {"ff1340557b325471f87873b8ec4a0cc84786b1496485b674145732e5d1b405e5"};
    const int srcTxOutputIndex {0};
    const string targetAddr {"mr4KnTn1ynJnX3BW4WaudRCgmYCqJjsPQz"};
    const uint64_t satoshisToTransfer {75000};

    construct_raw_transaction(privKeyWIF, srcTxId, srcTxOutputIndex, targetAddr, satoshisToTransfer);
}
