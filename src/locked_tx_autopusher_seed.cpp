#include <bitcoin/bitcoin.hpp>
#include "bing_client.hpp"
#include "redeem_script.hpp"
#include "funds_finder.hpp"
#include "purse_accessor.hpp"


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



/**
 * @param src_addr
 * @param priv_key_ec
 * @param satoshis_to_transfer
 * @param satoshis_fee
 * @param lock_until
 * @return transaction id of the created transaction
 */
string construct_p2sh_time_locking_transaction2(
        const string src_addr,
        const ec_private priv_key_ec,
        const uint64_t satoshis_to_transfer,
        const uint64_t satoshis_fee,
        const uint32_t lock_until
){
    BingClient bing_client;
    bing_client.init();

    const wallet::ec_public pub_key = priv_key_ec.to_public();
    const libbitcoin::config::base16 priv_key = libbitcoin::config::base16(priv_key_ec.secret());
    data_chunk pub_key_data_chunk;
    pub_key.to_data(pub_key_data_chunk);

    cout << "priv WIF: " << priv_key_ec << endl;
    cout << "public hex: " << pub_key << endl;
    cout << "private hex: " << priv_key << endl;

    cout << "fetch height: " << bing_client.fetch_height() << "\n";

    auto points_value = bing_client.fetch_utxo(payment_address(src_addr), 1, wallet::select_outputs::algorithm::individual);
    auto satoshis_needed = satoshis_to_transfer + satoshis_fee;
    auto utxos_funds = FundsFinder::find_funds(satoshis_needed, points_value);
    auto utxos = utxos_funds.first;
    auto available_funds = utxos_funds.second;
    if (utxos_funds.first.empty()){
        cout << "Insufficient funds, required " << satoshis_needed << ", available " << available_funds << "\n";
        return "";
    }
    auto refund = available_funds - satoshis_needed;
    cout << "available funds: " << available_funds << "\n";
    cout << "requested funds: " << satoshis_to_transfer << "\n";
    cout << "fee: " << satoshis_fee << "\n";
    cout << "refund: " << refund << "\n";


    // output 0
    script cltv_script = RedeemScript::to_pay_key_hash_pattern_with_lock(pub_key_data_chunk, lock_until);
    if(cltv_script.is_valid())
    {
        std::cout << "CLTV Script is Valid!" << std::endl;
    }else{
        std::cout << "CLTV Script Invalid!" << std::endl;
    }
    short_hash script_hash = bitcoin_short_hash(cltv_script.to_data(0));
    script pay_2_script_hash_locking_script = script(cltv_script.to_pay_script_hash_pattern(script_hash));
    output output0(satoshis_to_transfer, pay_2_script_hash_locking_script);

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
        script previous_locking_script = script().to_pay_key_hash_pattern(bitcoin_short_hash(pub_key_data_chunk));
        endorsement sig;
        if(previous_locking_script.create_endorsement(sig, priv_key_ec.secret(), previous_locking_script, tx, i, all))
        {
            std::cout << "Signature: " << encode_base16(sig) << std::endl;
        }
        // unlocking previous
        operation::list sig_script;
        sig_script.push_back(operation(sig));
        sig_script.push_back(operation(pub_key_data_chunk));
        script script_unlocking_previous_locking_script(sig_script);

        tx.inputs()[i].set_script(script_unlocking_previous_locking_script);
    }
    std::cout << "Raw Transaction with frozen output until " << lock_until << ":" << std::endl;
    std::cout << encode_base16(tx.to_data()) << std::endl;
    return encode_hash(tx.hash());
}

int main() {
    const uint64_t satoshis_to_transfer {4000};
    const uint64_t satoshis_fee {3000};
    const uint32_t lock_until = 1616108400;
    const string seedPhrase {"effort canal zoo clown shoulder genuine penalty moral unit skate few quick"};


    uint64_t required_funds{satoshis_to_transfer + satoshis_fee};
    BingClient bing_client;
    bing_client.init();

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

    vector<string> addresses;
    map<string, ec_private> addresses_to_ec_private;

    // from m/0/0 to m/0/99
    cout << "from m/0/0 to m/0/99: " << "\n";
    for (int i = 0; i < 100; ++i){
        hd_private hdPrivate = m0.derive_private(i);
        const payment_address address({ hdPrivate.secret(), payment_address::testnet_p2kh });
        cout << "m/0/" << i <<" address: " << address.encoded() << "\n";
        addresses.push_back(address.encoded());
        addresses_to_ec_private[address.encoded()] = ec_private(hdPrivate.secret(), payment_address::testnet_p2kh);
    }

    cout << "required funds: " << required_funds << "\n";
    AddressFunds funds = PurseAccessor::look_for_funds(bing_client, required_funds, addresses);

    cout << "funds found:" << "\n";
    cout << "address = " << funds.address << "\n";
    cout << "requested funds = " << funds.requested_funds << "\n";
    cout << "actual funds = " << funds.actual_funds << "\n";
    cout << "number of inputs = " << funds.points.size() << "\n";

    string source_address = funds.address;
    ec_private private_key = addresses_to_ec_private[funds.address];

    string tx_hash = construct_p2sh_time_locking_transaction2(source_address, private_key, satoshis_to_transfer, satoshis_fee, lock_until);

    cout << "===== data to unlock: ====" << "\n";
    cout << "lock time: " << lock_until << "\n";
    cout << "private key of address: " << source_address << "\n";
    cout << "available amount: " << satoshis_to_transfer << "\n";
    cout << "from ^^ please subtract fee" << "\n";
    cout << "funding transaction id: " << tx_hash << "\n";
    cout << "desired target address where the unlocked funds will be transferred" << "\n";
    cout << "==========================" << "\n";

    return 0;
}
