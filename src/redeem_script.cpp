#include "redeem_script.hpp"

using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;

/**
 * locks funds until "lock_until" number of seconds since Jan 1st 1970
 */

operation::list
RedeemScript::to_pay_key_hash_pattern_with_lock(const data_chunk &public_key,
                                                const uint32_t lock_until) {
  vector<uint8_t> lock_until_array(4);
  serializer<vector<uint8_t>::iterator>(lock_until_array.begin())
      .write_4_bytes_little_endian(lock_until);

  return operation::list{{lock_until_array},
                         {opcode::checklocktimeverify},
                         {opcode::drop},
                         {public_key},
                         {opcode::checksig}};
}
