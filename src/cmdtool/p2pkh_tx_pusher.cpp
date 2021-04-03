#include "src/common/bing_common.hpp"
#include <bitcoin/bitcoin.hpp>
#include <boost/program_options.hpp>

using namespace boost::program_options;
using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;

/**
 * produces p2pkh transaction
 * no locking, no p2sh, just plan and simple p2pkh
 * does not give rest
 * single input, single output
 * single funding address
 */

void construct_p2pkh_tx_from_funding_tx(const string priv_key_wif,
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

int main2() {
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

    construct_p2pkh_tx_from_funding_tx(priv_key_wif, src_tx_id, src_tx_output_index,
                                target_addr, satoshis_to_transfer);
  return 0;
}

int main(int argc, char* argv[]) {
    try {
        string help_text = "\nYou can find funding transaction by: \n" \
                " 1) bx fetch-balance <funding_address>\n" \
                " 2) if the balance is sufficient, do:\n" \
                "    bx fetch-utxo <desired-amount-in-satoshis> <funding-address>\n" \
                " 3) choose one utxo and capture 'hash' as funding transaction id\n" \
                "    and 'index' as funding transaction output index (vout)\n\n" \
                "Note that the amount to transfer must be smaller than the available amount in utxo\n" \
                "so that the remainder can be used as a fee.\n" \
                "This program does not give change, you need to use up the entire amount\n" \
                "from the UTXO.\n" \
                "Private key can be found in your wallet, in Electrum, go to tab 'Addresses',\n" \
                "highlight the desired address, right click and choose `private key'.\n" \
                "Ignore script type part of the key, like 'p2pkh', copy only the key part.\n\n" \
                "This is an offline program, it produces transaction in a hex format that can be broadcast\n" \
                "using any means, for example via 'bx send-tx <tx>' or any online transaction\n" \
                "broadcast drop-off place.\n\n";

        string priv_key_wif;
        string src_txid;
        int src_vout;
        uint64_t amount_to_transfer;
        string target_addr;
        options_description desc("Creates transaction to transfer funds via p2pkh\n\nRequired options");
        desc.add_options()
                ("help,h", "print usage message")
                ("priv-key,p", value<string>(&priv_key_wif)->required(), "private key (in WIF format)")
                ("txid,t", value<string>(&src_txid)->required(), "funding transaction id")
                ("vout,v", value<int>(&src_vout)->required(), "funding transaction output index (vout)")
                ("amount", value<uint64_t>(&amount_to_transfer)->required(), "amount to transfer (satoshis)")
                ("addr", value<string>(&target_addr)->required(), "target address")
                ;

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);

        if (vm.count("help") || argc <= 1){
            cout << "\n\n" << desc << "\n";
            cout << "example:" << "\n";
            cout << "--t=d001bd68fc87f05ae3760b4f9c4b64e1000d9194d9c95e0b5a7c7efd933f43d1 --v=0 --amount=890000 --p=<private-key> --addr=msWHhBL1vLycmZtQ5M1j7xWuUYvienydfq" << "\n";
            cout << help_text << "\n";
            return 1;
        }

        // note: must be after help option check
        notify(vm);

        construct_p2pkh_tx_from_funding_tx(priv_key_wif, src_txid, src_vout, target_addr, amount_to_transfer);

        return 0;
    }
    catch(exception& e) {
        cerr << e.what() << "\n";
    }
}