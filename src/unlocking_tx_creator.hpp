#ifndef UNLOCKING_TX_CREATOR_HPP
#define UNLOCKING_TX_CREATOR_HPP

#include <bitcoin/bitcoin.hpp>
#include <string>

class UnlockingTxCreator {
public:
  std::string create(const std::string priv_key_wif, const std::string src_tx_id,
                     const int src_tx_output_index, const uint32_t src_lock_until,
                     const std::string target_addr, const uint64_t satoshis_to_transfer);
};

#endif
