#ifndef REDEEM_SCRIPT_HPP
#define REDEEM_SCRIPT_HPP

#include <bitcoin/system.hpp>

class RedeemScript {
public:
    static libbitcoin::system::machine::operation::list to_pay_key_hash_pattern_with_lock(const libbitcoin::system::data_chunk& publicKey, const uint32_t lockUntil);
};


#endif
