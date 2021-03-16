#ifndef UNLOCKING_TX_CREATOR_HPP
#define UNLOCKING_TX_CREATOR_HPP

#include <bitcoin/system.hpp>
#include <string>

class UnlockingTxCreator {
public:
    std::string create(
            const std::string privKeyWIF,
            const std::string srcTxId,
            const int srcTxOutputIndex,
            const uint32_t srcLockUntil,
            const std::string targetAddr,
            const uint64_t satoshisToTransfer
            );
};

#endif
