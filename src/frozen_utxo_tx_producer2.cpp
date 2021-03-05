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
                    { opcode::push_size_4 },
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
        const string targetAddr,
        const uint64_t satoshisToTransfer,
        const uint32_t lockUntil
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
    script cltvScript = to_pay_key_hash_pattern_with_delay(payment_address(targetAddr).hash(), lockUntil);
    if(cltvScript.is_valid())
    {
        std::cout << "CLTV Script is Valid!" << std::endl;
    }else{
        std::cout << "CLTV Script Invalid!" << std::endl;
    }

    std::cout << "Redeem Script: " << std::endl;
    std::cout << cltvScript.to_string(0) << std::endl;
    std::cout << encode_base16(cltvScript.to_data(0)) <<std::endl;

    short_hash scriptHash = bitcoin_short_hash(cltvScript.to_data(0));
    script pay2ScriptHash = script(cltvScript.to_pay_script_hash_pattern(scriptHash));
    std::cout << "Locking Script: " << std::endl;
    std::cout << pay2ScriptHash.to_string(0xffffffff) << std::endl;

    output output1(satoshisToTransfer, pay2ScriptHash);

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
    std::cout << "Raw Transaction with frozen output until " << lockUntil << ":" << std::endl;
    std::cout << encode_base16(tx.to_data()) << std::endl;
}

int main() {
    /**
     * 1. private key for source_addr (note source address as SA)
     * 2. source transaction id (as found out via bx fetch-utxo <satoshis> SA)
     * 3. source transaction's output index (as found out via bx fetch-utxo <satoshis> SA)
     * 4. main target address
     * 5. amount to transfer in Satoshis
     * 6. lock until epoch time (in seconds)
     */
    const string privKeyWIF {"cSwx1yRAgjak6dBVcenZnHcfzNEXoeMBoRmq2PY79A4ABhjBPupd"}; // SA = mr4KnTn1ynJnX3BW4WaudRCgmYCqJjsPQz
    const string srcTxId {"83fd6c2dbc1dad77384bfb1007550998f48a2d20419d5e3ad0239e944f65f21c"}; // bx fetch-utxo 77000 mr4KnTn1ynJnX3BW4WaudRCgmYCqJjsPQz
    const int srcTxOutputIndex {0};
    const string targetAddr {"mroQ8Hpydtv5p4mpF7Tbtjqkwi7wHXYZuR"};
    const uint64_t satoshisToTransfer {86500};
    const uint32_t lockUntil = 1614984000;

    construct_raw_transaction(privKeyWIF, srcTxId, srcTxOutputIndex, targetAddr, satoshisToTransfer, lockUntil);
}
