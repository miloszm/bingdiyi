/**
 * Copyright (c) 2020-2021 bingdiyi developers (see AUTHORS)
 *
 * This file is part of bingdiyi.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "src/common/bing_common.hpp"
#include <bitcoin/system.hpp>
#include <binglib/redeem_script.hpp>
#include <boost/program_options.hpp>

using namespace boost::program_options;
using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;

/**
 * spender
 *
 * spends (unlocks) funds locked via p2sh script containing OP_CLTV
 *
 */


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
    data_chunk pubKeyChunk;
    pubKey.to_data(pubKeyChunk);

    string hashString = srcTxId;
    hash_digest utxoHash;
    decode_hash(utxoHash, hashString);
    output_point utxo(utxoHash, srcTxOutputIndex);
    input input1 = input();
    input1.set_previous_output(utxo);
    input1.set_sequence(0);

    script currentLockingScript = script().to_pay_key_hash_pattern(payment_address(targetAddr).hash());
    output output1(satoshisToTransfer, currentLockingScript);

    transaction tx = transaction();
    tx.inputs().push_back(input1);
    tx.outputs().push_back(output1);
    tx.set_locktime(srcLockUntil);
    tx.set_version(1);


    //bx input-sign fd2fc82cc442f35b3b577dc8f300d80007cc53c8d3f922265ccdc84e5c2729d5 "[50714760] checklocktimeverify drop [0375253f2f96889d04eda186cfd2f0f161f4888e538066acb39adb1729ed374e4e] checksig" 010000000155eb2941e57ebf58b0296f114bad51c459e72df3308964ff9c95803fe91c49a80000000000000000000130570500000000001976a9147bc59a29fdd04f10d03ae5f3668a36163ffc580688ac50714760

    script redeemScript = RedeemScript::to_pay_key_hash_pattern_with_lock(pubKeyChunk, srcLockUntil);
    if(!redeemScript.is_valid())
    {
        std::cout << "CLTV Script Invalid!" << std::endl;
    }
    endorsement sig;
    if(script::create_endorsement(sig, privKeyEC.secret(), redeemScript, tx, 0u, all))
    {
        std::cout << "Signature: " << encode_base16(sig) << std::endl;
    }

    // bx input-set "zero [3044022024048cd26f0d493173c4c1e15be7fc4bb0c9f91bbba422d46d09b910ec28c0ac02202477ce166b20b13aee4e997c5e2ffbdd8d274d06066307690f75dd7dcc5a3a6a01] [04c4684560b175210314488ebfec9889c4253fe2d1a21715b932864d2892193e4ca60e0acbd1d9fd1dac]" 0100000001a626c94281c1eaa9f2c1589d82355285367e42259e3ece651d9ee736b790092a0000000000000000000100350c00000000001976a914fdbbbe6062fef2fca812e404e3dcb43dcdb4108888acc4684560

    operation::list sigScript;
    sigScript.push_back(operation(sig));
    sigScript.push_back(redeemScript.to_data(0));
    script scriptUnlockingPreviousLockingScript(sigScript);
    tx.inputs()[0].set_script(scriptUnlockingPreviousLockingScript);

    cout << "==========================" << "\n";
    cout << "==========================" << "\n";
    std::cout << "Transaction to be sent to unlock the funds: " << std::endl;
    std::cout << encode_base16(tx.to_data()) << std::endl;
    cout << "==========================" << "\n";
    cout << "==========================" << "\n";
}

int main(int argc, char* argv[]) {
    try {
        string help_text = "\nYou need to have the following items of information available\n" \
                "(as advised and provided by the locking program):\n" \
                " 1) lock time\n" \
                " 2) private key of the source address (from where the funds originated before locking)\n" \
                " 3) available amount (from which the fee needs to be subtracted)\n" \
                " 4) funding (locking) transaction id to unlock\n" \
                "You will also need the destination address where the funds should be transferred to.\n\n" \
                "An example unlocking information looks as follows:\n" \
                "===== data to unlock: ====\n" \
                "lock time: 1616418000\n" \
                "private key of address: mkP2QQqQYsReSpt3JBoRQ5zVdw3ra1jenh\n" \
                "available amount: 210000 satoshi\n" \
                "from ^^ please subtract fee\n" \
                "funding transaction id to unlock: 085f3e80771036a68ee4116fdb208eb44ffadce70fcd9d77cf935537535d0b27\n" \
                "desired destination address to which the unlocked funds will be transferred\n" \
                "==========================\n\n" \
                "This program produces transaction in a hex format that can be broadcast\n" \
                "using any means, for example via 'bx send-tx <tx>' or any online transaction\n" \
                "broadcast drop-off place.\n\n" \
                "This program works for both mainnet and testnet.\n\n";
        string priv_key_wif;
        string src_txid;
        int src_vout {0};
        uint64_t amount_to_transfer;
        uint32_t lock_until;
        string target_addr;
        options_description desc("Creates transaction to unlock funds locked via p2sh\n\nRequired options");
        desc.add_options()
                ("help,h", "print usage message")
                ("priv-key,p", value<string>(&priv_key_wif)->required(), "private key to unlock the funding transaction (in WIF format)")
                ("funding-txid,f", value<string>(&src_txid)->required(), "funding transaction id")
                ("amount", value<uint64_t>(&amount_to_transfer)->required(), "amount to transfer (satoshis)")
                ("lock-until,l", value<uint32_t>(&lock_until)->required(), "lock until epoch time (seconds)")
                ("dest,d", value<string>(&target_addr)->required(), "destination address")
                ;

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);

        if (vm.count("help") || argc <= 1){
            cout << "\n\n" << desc << "\n";
            cout << "example:" << "\n";
            cout << "--p=<private-key> --f=29e959ce847842ee86d22703b68c725c854328675b660f15d5272fa71ffc38ba --a=88000 --l=1616255893 --d=n4XMrFeDdEg2vtDYQzDcaK7jcthh5xG4MX" << "\n";
            cout << help_text << "\n";
            return 1;
        }

        // note: must be after help option check
        notify(vm);

        construct_raw_transaction(priv_key_wif, src_txid, src_vout, lock_until, target_addr, amount_to_transfer);

        return 0;
    }
    catch(exception& e) {
        cerr << e.what() << "\n";
    }
}
