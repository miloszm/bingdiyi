#ifndef UNLOCKING_TX_CREATOR_HPP
#define UNLOCKING_TX_CREATOR_HPP

#include <bitcoin/bitcoin.hpp>
#include <string>

using namespace std;

class UnlockingTxCreator {
public:
    string create(
            const string privKeyWIF,
            const string srcTxId,
            const int srcTxOutputIndex,
            const uint32_t srcLockUntil,
            const string targetAddr,
            const uint64_t satoshisToTransfer
            );
};

#endif
