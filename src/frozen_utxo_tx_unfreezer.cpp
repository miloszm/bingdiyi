#include <bitcoin/bitcoin.hpp>

using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;


operation::list to_pay_key_hash_pattern_with_delay(const short_hash& hash, const uint32_t lockUntil)
{
    vector<uint8_t> lockUntilArray(4);
    serializer<vector<uint8_t>::iterator>(lockUntilArray.begin()).write_4_bytes_little_endian(lockUntil);

    return operation::list
            {
                    { lockUntilArray },
                    { opcode::checklocktimeverify },
                    { opcode::drop },
                    { opcode::dup },
                    { opcode::hash160 },
                    { to_chunk(hash) },
                    { opcode::equalverify },
                    { opcode::checksig }
            };
}

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

    cout << "priv WIF: " << privKeyEC << endl;
    cout << "public hex: " << pubKey << endl;
    cout << "private hex: " << privKey << endl;

    /**
     * make output
     * payment_address decodes base58 address and calculates hash for it
     * to_pay_key_hash_pattern then creates the script, filling out the address
     */
    script outputScript = to_pay_key_hash_pattern_with_delay(payment_address(targetAddr).hash(), srcLockUntil);
    output output1(satoshisToTransfer, outputScript);

    /**
     * make utxo
     * funding transaction id is decoded from hex string into bytes and reversed
     * hash_digest is simply an array of 32 bytes
     */
    string hashString = srcTxId;
    hash_digest utxoHash;
    decode_hash(utxoHash, hashString);
    output_point utxo(utxoHash, srcTxOutputIndex);

    /**
     * previous locking script
     * convert public key to chunk data, and then calculate sha 256 and ripemd160 on it
     * then it is being placed in a locking script created from scratch
     */
    data_chunk pubKeyChunk;
    pubKey.to_data(pubKeyChunk);
    script lockingScript = script().to_pay_key_hash_pattern(bitcoin_short_hash(pubKeyChunk));
    std::cout << "\nPrevious Locking Script: " << lockingScript.to_string(0xffffffff) << std::endl;

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
     * endorsement is really a hashed signature of data given in
     */
    endorsement sig;
    if(lockingScript.create_endorsement(sig, privKeyEC.secret(), lockingScript, tx, 0u, all))
    {
        std::cout << "Signature: " << std::endl;
        std::cout << encode_base16(sig) << "\n" << std::endl;
    }

    /**
     * make Sig Script
     * unlocking script is created from scratch here as a list of operations
     */
    operation::list sigScript;
    sigScript.push_back(operation(sig));
    sigScript.push_back(operation(pubKeyChunk));
    script unlockingScript(sigScript);
    std::cout << "\nUnlocking Script: " << unlockingScript.to_string(0xffffffff) << std::endl;

    /**
     * make Signed TX
     * fill out input with unlocking script which was missing until this point
     */
    tx.inputs()[0].set_script(unlockingScript);
    std::cout << "Raw Transaction: " << std::endl;
    std::cout << encode_base16(tx.to_data()) << std::endl;
}

int main() {
    /**
     *
     * 1. private key for source_addr
     * 2. source transaction id (as found out via bx fetch-utxo)
     * 3. source transaction's output index (as found out via bx fetch-utxo)
     * 4. source lock time in epoch (to be able to correctly recreate locking script and sign it)
     * 5. main target address
     * 6. amount to transfer in Satoshis
     */
    const string privKeyWIF {"cQjiP56Zt8hiv6oMUyGdcgQiGYmHAf5gMJdsHXJfyu82xX3ijkDN"}; // mhM6yUngQwzfvqRZYFpDnkfHhLtBGieUxn
    const string srcTxId {"4cd690b02c32cdcfbe20e7f9014a6a95a46c9ea98cc10e5bb4a21068f5bb6cb2"};
    const int srcTxOutputIndex {0};
    const uint32_t srcLockUntil {1614898838};
    const string targetAddr {"n4SdAqMKRPk48fHP9coCcBN9dxbcVmw2S3"};
    const uint64_t satoshisToTransfer {65000};

    construct_raw_transaction(privKeyWIF, srcTxId, srcTxOutputIndex, srcLockUntil, targetAddr, satoshisToTransfer);
}
