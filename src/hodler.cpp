#include <bitcoin/bitcoin.hpp>

using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;


operation::list redeemScript(const data_chunk& publicKey, const uint32_t lockUntil)
{
    vector<uint8_t> lockUntilArray(4);
    serializer<vector<uint8_t>::iterator>(lockUntilArray.begin()).write_4_bytes_little_endian(lockUntil);

    return operation::list
            {
                    { lockUntilArray },
                    { opcode::checklocktimeverify },
                    { opcode::drop },
                    { publicKey },
                    { opcode::checksig }
            };
}


void createP2shAddress(const wallet::ec_public& pubKey, const uint32_t lockUntil)
{
    data_chunk pubKeyDataChunk;
    pubKey.to_data(pubKeyDataChunk);
    script cltvScript = redeemScript(pubKeyDataChunk, lockUntil);
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
    std::cout << "Redeem Script Hash: " << libbitcoin::config::base16(scriptHash) << std::endl;
    script pay2ScriptHashLockingScript = script(cltvScript.to_pay_script_hash_pattern(scriptHash));
    std::cout << "Locking Script: " << std::endl;
    std::cout << pay2ScriptHashLockingScript.to_string(0xffffffff) << std::endl;
    std::cout << "Locking Script hex: " << std::endl;
    std::cout << encode_base16(pay2ScriptHashLockingScript.to_data(0)) << std::endl;
}

int main()
{
    const string privKeyWIF {"cNJLkBWo6pe4qEuYuTvU6DcNnyDBeZ7qET8vwc4HZM4RYawmw9xk"}; // SA = mmhixqxrCnb69iMZLwDaQZmTY7g7JQMkoh
    const wallet::ec_private privKeyEC(privKeyWIF);
    const wallet::ec_public pubKey = privKeyEC.to_public();
    const uint32_t lockUntil = 1615161540;

    createP2shAddress(pubKey, lockUntil);

    cout << "should be:" << "\n";
    cout << "a9142c135b63577126ac7164804aa40eb148ce93417387" << "\n";
}