#include "unlocking_tx_creator.hpp"
#include "redeem_script.hpp"

using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;

string UnlockingTxCreator::create(const string priv_key_wif,
                                  const string src_tx_id,
                                  const int src_tx_output_index,
                                  const uint32_t src_lock_until,
                                  const string target_addr,
                                  const uint64_t satoshis_to_transfer) {
  const wallet::ec_private priv_key_ec(priv_key_wif);
  const wallet::ec_public pub_key = priv_key_ec.to_public();
  const libbitcoin::config::base16 priv_key =
      libbitcoin::config::base16(priv_key_ec.secret());
  data_chunk pub_key_chunk;
  pub_key.to_data(pub_key_chunk);

  cout << "priv WIF: " << priv_key_ec << endl;
  cout << "public hex: " << pub_key << endl;
  cout << "private hex: " << priv_key << endl;

  // bx tx-encode -i
  // 2a0990b736e79e1d65ce3e9e25427e36855235829d58c1f2a9eac18142c926a6:0:0 -o
  // n4eaAFB3GPmrJR4ummYpQmYTx2VaNftuPe:800000 -l 1615161540

  hash_digest utxo_hash;
  decode_hash(utxo_hash, src_tx_id); // reverse only
  output_point utxo(utxo_hash, src_tx_output_index);
  input input1 = input();
  input1.set_previous_output(utxo);
  input1.set_sequence(0);

  script current_locking_script =
      script().to_pay_key_hash_pattern(payment_address(target_addr).hash());
  output output1(satoshis_to_transfer, current_locking_script);

  transaction tx = transaction();
  tx.inputs().push_back(input1);
  tx.outputs().push_back(output1);
  tx.set_locktime(src_lock_until);
  tx.set_version(1);

  // bx input-sign
  // fd2fc82cc442f35b3b577dc8f300d80007cc53c8d3f922265ccdc84e5c2729d5
  // "[50714760] checklocktimeverify drop
  // [0375253f2f96889d04eda186cfd2f0f161f4888e538066acb39adb1729ed374e4e]
  // checksig"
  // 010000000155eb2941e57ebf58b0296f114bad51c459e72df3308964ff9c95803fe91c49a80000000000000000000130570500000000001976a9147bc59a29fdd04f10d03ae5f3668a36163ffc580688ac50714760

  cout << "input to sig tx " << encode_base16(tx.to_data()) << "\n";
  cout << "should          "
       << "010000000155eb2941e57ebf58b0296f114bad51c459e72df3308964ff9c95803fe9"
          "1c49a80000"
          "000000000000000130570500000000001976a9147bc59a29fdd04f10d03ae5f3668a"
          "36163ffc58"
          "0688ac50714760"
       << "\n";

  script redeemScript = RedeemScript::to_pay_key_hash_pattern_with_lock(
      pub_key_chunk, src_lock_until);
  if (redeemScript.is_valid()) {
    std::cout << "CLTV Script is Valid!" << std::endl;
  } else {
    std::cout << "CLTV Script Invalid!" << std::endl;
  }
  endorsement sig;
  if (script::create_endorsement(sig, priv_key_ec.secret(), redeemScript, tx,
                                 0u, all)) {
    std::cout << "Signature: " << encode_base16(sig) << std::endl;
  }

  // bx input-set "zero
  // [3044022024048cd26f0d493173c4c1e15be7fc4bb0c9f91bbba422d46d09b910ec28c0ac02202477ce166b20b13aee4e997c5e2ffbdd8d274d06066307690f75dd7dcc5a3a6a01]
  // [04c4684560b175210314488ebfec9889c4253fe2d1a21715b932864d2892193e4ca60e0acbd1d9fd1dac]"
  // 0100000001a626c94281c1eaa9f2c1589d82355285367e42259e3ece651d9ee736b790092a0000000000000000000100350c00000000001976a914fdbbbe6062fef2fca812e404e3dcb43dcdb4108888acc4684560

  operation::list sig_script;
  sig_script.push_back(operation(sig));
  sig_script.push_back(redeemScript.to_data(0));
  script script_unlocking_previous_locking_script(sig_script);
  tx.inputs()[0].set_script(script_unlocking_previous_locking_script);

  std::cout << "Raw Transaction: " << std::endl;
  std::cout << encode_base16(tx.to_data()) << std::endl;
  return encode_base16(tx.to_data());
}
