#ifndef ONLINE_P2PKH_TX_CREATOR_HPP
#define ONLINE_P2PKH_TX_CREATOR_HPP

#include <bitcoin/bitcoin.hpp>
#include <string>

using namespace std;

class OnlineP2pkhTxCreator {
public:
    static string construct_p2pkh_tx_from_address(
            const std::string src_addr,
            const std::string priv_key_wif,
            const uint64_t amount_to_transfer,
            const uint64_t satoshis_fee,
            const std::string target_addr
    );

    static string construct_p2pkh_tx_from_address(
            const std::string src_addr,
            const bc::wallet::ec_private priv_key_ec,
            const uint64_t amount_to_transfer,
            const uint64_t satoshis_fee,
            const std::string target_addr
    );
};


#endif