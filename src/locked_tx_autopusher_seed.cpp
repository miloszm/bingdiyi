#include <bitcoin/bitcoin.hpp>
#include "bing_client.hpp"
#include "redeem_script.hpp"
#include "funds_finder.hpp"


using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;


/**
 * autopusher2
 *
 * has the ability to extract funding transactions for a given address
 * support multiple inputs if needed
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
    BingClient bingClient;
    bingClient.init();

    const wallet::ec_private privKeyEC(privKeyWIF);
    const wallet::ec_public pubKey = privKeyEC.to_public();
    const libbitcoin::config::base16 privKey = libbitcoin::config::base16(privKeyEC.secret());
    data_chunk pubKeyDataChunk;
    pubKey.to_data(pubKeyDataChunk);

    cout << "priv WIF: " << privKeyEC << endl;
    cout << "public hex: " << pubKey << endl;
    cout << "private hex: " << privKey << endl;

    cout << "fetch height: " << bingClient.fetch_height() << "\n";

    auto pointsValue = bingClient.fetch_utxo(payment_address(srcAddr), 1, wallet::select_outputs::algorithm::individual);
    auto satoshisNeeded = satoshisToTransfer + satoshisFee;
    auto utxosFunds = FundsFinder::find_funds(satoshisNeeded, pointsValue);
    auto utxos = utxosFunds.first;
    auto availableFunds = utxosFunds.second;
    if (utxosFunds.first.empty()){
        cout << "Insufficient funds, required " << satoshisNeeded << ", available " << availableFunds << "\n";
        return;
    }
    auto refund = availableFunds - satoshisNeeded;
    cout << "available funds: " << availableFunds << "\n";
    cout << "requested funds: " << satoshisToTransfer << "\n";
    cout << "fee: " << satoshisFee << "\n";
    cout << "refund: " << refund << "\n";


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

    // tx
    transaction tx = transaction();
    for(auto utxo: utxos){
        input input1 = input();
        input1.set_previous_output(utxo);
        input1.set_sequence(0xfffffffe);
        tx.inputs().push_back(input1);
    }
    tx.outputs().push_back(output0);
    if (refund > 0){
        output output1(refund, script().to_pay_key_hash_pattern(payment_address(srcAddr).hash()));
        tx.outputs().push_back(output1);
    }
    tx.set_version(1);

    // set unlocking script in inputs
    for (unsigned int i = 0; i < utxos.size(); ++i) {
        // sig
        script previousLockingScript = script().to_pay_key_hash_pattern(bitcoin_short_hash(pubKeyDataChunk));
        endorsement sig;
        if(previousLockingScript.create_endorsement(sig, privKeyEC.secret(), previousLockingScript, tx, i, all))
        {
            std::cout << "Signature: " << encode_base16(sig) << std::endl;
        }
        // unlocking previous
        operation::list sigScript;
        sigScript.push_back(operation(sig));
        sigScript.push_back(operation(pubKeyDataChunk));
        script scriptUnlockingPreviousLockingScript(sigScript);

        tx.inputs()[i].set_script(scriptUnlockingPreviousLockingScript);
    }
    std::cout << "Raw Transaction with frozen output until " << lockUntil << ":" << std::endl;
    std::cout << encode_base16(tx.to_data()) << std::endl;
}

int main2() {
    const string version {"0.001"};
    cout << "locked_tx_pusher" << "\n";
    cout << "version:" << version << "\n";

    const string srcAddr {"mkP2QQqQYsReSpt3JBoRQ5zVdw3ra1jenh"};
    const string privKeyWIF {"cQZ57Q5w1F9YS5n1h81QqnrN2Ea54BMNPCnzoqqgPMdB9wbzwxM6"};
    const uint64_t satoshisToTransfer {2000000};
    const uint64_t satoshisFee {10000};
    const uint32_t lockUntil = 1615591800;

    construct_p2sh_time_locking_transaction(srcAddr, privKeyWIF, satoshisToTransfer, satoshisFee, lockUntil);
}


int main() {
    const string seedPhrase {"effort canal zoo clown shoulder genuine penalty moral unit skate few quick"};

    const word_list mnemonic = split(seedPhrase, " ");

    if (electrum::validate_mnemonic(mnemonic, language::en)){
        cout << "mnemonic validated OK" << "\n";
    } else {
        cout << "mnemonic BAD" << "\n";
    }

    long_hash seed = electrum::decode_mnemonic(mnemonic);

    cout << "seed=" << "\n";
    cout << config::base16(seed) << "\n";

    data_chunk seedAsChunk(seed.begin(), seed.end());

    const hd_private m(seedAsChunk, hd_private::testnet);
    const hd_public m_pub = m;

    auto m0_pub = m.derive_public(0);
    auto m1_pub = m.derive_public(1);

    cout << m_pub.encoded() << "\n";
    cout << m0_pub.encoded() << "\n";
    cout << m1_pub.encoded() << "\n";

    hd_private m0 = m.derive_private(0);

    // from m/0/0 to m/0/29
    cout << "from m/0/0 to m/0/29: " << "\n";
    for (int i = 0; i < 30; ++i){
        hd_private hdPrivate = m0.derive_private(i);
        const payment_address address({ hdPrivate.secret(), payment_address::testnet_p2kh });
        cout << "m/0/" << i <<" address: " << address.encoded() << "\n";
    }
    return 0;
}
