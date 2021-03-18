#include <bitcoin/bitcoin.hpp>

using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;

/**
 * produces p2pkh transaction
 * no locking, no p2sh, just plan and simple p2pkh
 */

void construct_raw_transaction(const string priv_key_wif,
                               const string src_tx_id,
                               const int src_tx_output_index,
                               const string target_addr,
                               const uint64_t satoshis_to_transfer) {
  const wallet::ec_private priv_key_ec(priv_key_wif);
  const wallet::ec_public pub_key = priv_key_ec.to_public();
  const libbitcoin::config::base16 priv_key =
      libbitcoin::config::base16(priv_key_ec.secret());

  cout << "priv WIF: " << priv_key_ec << endl;
  cout << "public hex: " << pub_key << endl;
  cout << "private hex: " << priv_key << endl;

  /**
   * make output
   * payment_address decodes base58 address and calculates hash for it
   * to_pay_key_hash_pattern then creates the script, filling out the address
   */
  script current_locking_script =
      script().to_pay_key_hash_pattern(payment_address(target_addr).hash());
  output output1(satoshis_to_transfer, current_locking_script);

  /**
   * make utxo
   * funding transaction id is decoded from hex string into bytes and reversed
   * (hash_digest is a plain array of 32 bytes)
   */
  hash_digest utxo_hash;
  decode_hash(utxo_hash, src_tx_id); // reverse
  output_point utxo(utxo_hash, src_tx_output_index);

  /**
   * previous locking script
   * convert public key to chunk data, and then calculate sha 256 and ripemd160
   * on it then place it in a locking script created from scratch previous
   * locking script is not read, but rather recreated from scratch here it is
   * needed for signing only, otherwise it is not used
   */
  data_chunk pub_key_chunk;
  pub_key.to_data(pub_key_chunk);
  script previous_locking_script =
      script().to_pay_key_hash_pattern(bitcoin_short_hash(pub_key_chunk));
  std::cout << "\nPrevious Locking Script: "
            << previous_locking_script.to_string(0xffffffff) << std::endl;

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
   * endorsement is created out of the private key, locking script, transaction,
   * input index endorsement is a hashed signature of provided data
   */
  endorsement sig;
  if (previous_locking_script.create_endorsement(
          sig, priv_key_ec.secret(), previous_locking_script, tx, 0u, all)) {
    std::cout << "Signature: " << encode_base16(sig) << std::endl;
  }

  /**
   * make Sig Script
   * unlocking script is created from scratch here as a list of operations
   */
  operation::list sig_script;
  sig_script.push_back(operation(sig));
  sig_script.push_back(operation(pub_key_chunk));
  script script_unlocking_previous_locking_script(sig_script);
  std::cout << "\nUnlocking Script: "
            << script_unlocking_previous_locking_script.to_string(0xffffffff)
            << std::endl;

  /**
   * make Signed TX
   * fill out input with unlocking script which was missing until this point
   */
  tx.inputs()[0].set_script(script_unlocking_previous_locking_script);
  std::cout << "Raw Transaction: " << std::endl;
  std::cout << encode_base16(tx.to_data()) << std::endl;
}

int main() {
  /**
   * 1. private key for source_addr (note source address as SA)
   * 2. source transaction id (as found out via bx fetch-utxo <satoshis> SA)
   * 3. source transaction's output index (as found out via bx fetch-utxo
   * <satoshis> SA)
   * 4. main target address
   * 5. amount to transfer in Satoshis
   */
  const string priv_key_wif{
      "cSv9QafnL7UxFDdbRe7G9JtzWn3RoV1GCW9FfFzjDgLUNZgsBwsA"}; // SA = n2JZCSr8MeGuGtvRVjZTqgNNw9pyYW98Pm
  const string src_tx_id{
      "ff1340557b325471f87873b8ec4a0cc84786b1496485b674145732e5d1b405e5"};
  const int src_tx_output_index{0};
  const string target_addr{"mr4KnTn1ynJnX3BW4WaudRCgmYCqJjsPQz"};
  const uint64_t satoshis_to_transfer{75000};

  construct_raw_transaction(priv_key_wif, src_tx_id, src_tx_output_index,
                            target_addr, satoshis_to_transfer);
}
