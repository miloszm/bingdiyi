#include <bitcoin/system.hpp>
#include "redeem_script.hpp"

using namespace std;
using namespace bc;
using namespace bc::system;
using namespace bc::system::chain;
using namespace bc::system::wallet;
using namespace bc::system::machine;


/**
 * locks funds until "lockUntil" number of seconds since Jan 1st 1970
 */

operation::list RedeemScript::to_pay_key_hash_pattern_with_lock(const data_chunk& publicKey, const uint32_t lockUntil)
{
    vector<uint8_t> lockUntilArray(4);
    serializer<vector<uint8_t>::iterator>(lockUntilArray.begin()).write_4_bytes_little_endian(lockUntil);

    return operation::list
            {
                    { lockUntilArray },
                    { opcode::checklocktimeverify },
                    { opcode::drop },
                    { publicKey },
                    { opcode::checksig }
            };
}
