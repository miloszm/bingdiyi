#include <bitcoin/bitcoin.hpp>
#include "bing_client.hpp"
#include "redeem_script.hpp"

using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;


/**
 * Creates list of utxos to provide funds needed
 * @param satoshisNeeded
 * @param points
 * @return pair of: 1) list of utxos 2) gathered funds
 * if 1) empty means funds were not sufficient, in such case 2) contains available funds
 */
pair<vector<output_point>, uint64_t> find_funds(uint64_t satoshisNeeded, chain::points_value points){
    vector<output_point> utxos;
    uint64_t gatheredFunds {0};
    for (auto p = begin(points.points); p != end(points.points) && gatheredFunds < satoshisNeeded; ++p){
        if (points.value() > 0){
            output_point utxo(p->hash(), p->index());
            utxos.push_back(utxo);
            gatheredFunds += p->value();
        }
    }
    if (gatheredFunds < satoshisNeeded)
        return {vector<output_point>(), gatheredFunds};
    else
        return {utxos, gatheredFunds};
}


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
//    hash_digest srcTxIdDataReversed;
//    decode_hash(srcTxIdDataReversed, srcTxId);

    cout << "priv WIF: " << privKeyEC << endl;
    cout << "public hex: " << pubKey << endl;
    cout << "private hex: " << privKey << endl;

    cout << "fetch height: " << bingClient.fetchHeight() << "\n";

    auto pointsValue = bingClient.fetchUtxo(payment_address(srcAddr), 1, wallet::select_outputs::algorithm::individual);
    auto satoshisNeeded = satoshisToTransfer + satoshisFee;
    auto utxosFunds = find_funds(satoshisNeeded, pointsValue);
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

    // set unlocking script in inputs
    for (int i = 0; i < utxos.size(); ++i)
        tx.inputs()[i].set_script(scriptUnlockingPreviousLockingScript);
    std::cout << "Raw Transaction with frozen output until " << lockUntil << ":" << std::endl;
    std::cout << encode_base16(tx.to_data()) << std::endl;
}

int main() {
    const string version {"0.001"};
    cout << "locked_tx_pusher" << "\n";
    cout << "version:" << version << "\n";

    const string srcAddr {"mkP2QQqQYsReSpt3JBoRQ5zVdw3ra1jenh"};
    const string privKeyWIF {"cQZ57Q5w1F9YS5n1h81QqnrN2Ea54BMNPCnzoqqgPMdB9wbzwxM6"};
    const uint64_t satoshisToTransfer {2000000};
    const uint64_t satoshisFee {10000};
    const uint32_t lockUntil = 1615500000;

    construct_p2sh_time_locking_transaction(srcAddr, privKeyWIF, satoshisToTransfer, satoshisFee, lockUntil);
}