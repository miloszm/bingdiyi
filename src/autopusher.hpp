#ifndef AUTOPUSHER_HPP
#define AUTOPUSHER_HPP

#include <bitcoin/bitcoin.hpp>


void construct_p2sh_time_locking_transaction_from_address(
        const std::string src_addr,
        const std::string priv_key_wif,
        const uint64_t amount_to_transfer,
        const uint64_t satoshis_fee,
        const uint32_t lock_until
);

void construct_p2sh_time_locking_transaction_from_address(
        const std::string src_addr,
        const bc::wallet::ec_private priv_key_ec,
        const uint64_t amount_to_transfer,
        const uint64_t satoshis_fee,
        const uint32_t lock_until
);


#endif