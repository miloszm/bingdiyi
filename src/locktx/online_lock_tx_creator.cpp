#include "src/common/bing_common.hpp"
#include <bitcoin/bitcoin.hpp>
#include "src/libbitcoinclient/libb_client.hpp"
#include "src/libbscript/redeem_script.hpp"
#include "src/libbfunds/funds_finder.hpp"
#include "online_lock_tx_creator.hpp"


using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;


void OnlineLockTxCreator::construct_p2sh_time_locking_transaction_from_address(
        const string src_addr,
        const string priv_key_wif,
        const uint64_t amount_to_transfer,
        const uint64_t satoshis_fee,
        const uint32_t lock_until
){
    const wallet::ec_private priv_key_ec(priv_key_wif);
    return construct_p2sh_time_locking_transaction_from_address(src_addr, priv_key_ec, amount_to_transfer, satoshis_fee, lock_until);
}


void OnlineLockTxCreator::construct_p2sh_time_locking_transaction_from_address(
        const string src_addr,
        const ec_private priv_key_ec,
        const uint64_t amount_to_transfer,
        const uint64_t satoshis_fee,
        const uint32_t lock_until
){
    LibbClient libb_client;
    libb_client.init();

    const wallet::ec_public pub_key = priv_key_ec.to_public();
    const libbitcoin::config::base16 priv_key = libbitcoin::config::base16(priv_key_ec.secret());
    data_chunk pub_key_data_chunk;
    pub_key.to_data(pub_key_data_chunk);

    auto points_value = libb_client.fetch_utxo(payment_address(src_addr), 1, wallet::select_outputs::algorithm::individual);
    auto satoshis_needed = amount_to_transfer + satoshis_fee;
    auto utxos_funds = FundsFinder::find_funds(satoshis_needed, points_value);
    auto utxos = utxos_funds.first;
    auto available_funds = utxos_funds.second;
    if (utxos_funds.first.empty()){
        cout << "Insufficient funds, required " << satoshis_needed << ", maximum at one address available " << available_funds << "\n";
        return;
    }
    auto refund = available_funds - satoshis_needed;

    // output 0
    script cltv_script = RedeemScript::to_pay_key_hash_pattern_with_lock(pub_key_data_chunk, lock_until);
    if(!cltv_script.is_valid()){
        std::cout << "CLTV Script Invalid!" << std::endl;
    }
    short_hash script_hash = bitcoin_short_hash(cltv_script.to_data(0));
    script pay2ScriptHashLockingScript = script(cltv_script.to_pay_script_hash_pattern(script_hash));
    output output0(amount_to_transfer, pay2ScriptHashLockingScript);

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
        output output1(refund, script().to_pay_key_hash_pattern(payment_address(src_addr).hash()));
        tx.outputs().push_back(output1);
    }
    tx.set_version(1);

    // set unlocking script in inputs
    for (unsigned int i = 0; i < utxos.size(); ++i) {
        // sig
        script previousLockingScript = script().to_pay_key_hash_pattern(bitcoin_short_hash(pub_key_data_chunk));
        endorsement sig;
        if(previousLockingScript.create_endorsement(sig, priv_key_ec.secret(), previousLockingScript, tx, i, all))
        {
            std::cout << "Signature: " << encode_base16(sig) << std::endl;
        }
        // unlocking previous
        operation::list sigScript;
        sigScript.push_back(operation(sig));
        sigScript.push_back(operation(pub_key_data_chunk));
        script scriptUnlockingPreviousLockingScript(sigScript);

        tx.inputs()[i].set_script(scriptUnlockingPreviousLockingScript);
    }

    cout << "==========================" << "\n";
    cout << "==========================" << "\n";
    std::cout << "Transaction with frozen output until " << lock_until << ":" << std::endl;
    std::cout << encode_base16(tx.to_data()) << std::endl;
    cout << "==========================" << "\n";
    cout << "==========================" << "\n";

    string tx_to_unlock = encode_hash(tx.hash());

    cout << "===== data that will be needed to unlock the funds: ====" << "\n";
    cout << "1) lock time: " << lock_until << "\n";
    cout << "2) private key of address: " << src_addr << "\n";
    cout << "3) available amount: " << amount_to_transfer << "\n";
    cout << "   from ^^ please subtract fee" << "\n";
    cout << "4) funding transaction id: " << tx_to_unlock << "\n";
    cout << "5) desired target address to which the unlocked funds will be transferred" << "\n";
    cout << "==========================" << "\n";
    cout << "==========================" << "\n";

}