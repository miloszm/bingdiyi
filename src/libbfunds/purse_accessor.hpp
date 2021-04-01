#ifndef PURSE_ACCESSOR_HPP
#define PURSE_ACCESSOR_HPP

#include <binglib/electrum_api_client.hpp>
#include <binglib/libb_client.hpp>
#include <bitcoin/bitcoin.hpp>

struct AddressFunds {
  std::string address;
  uint64_t requested_funds;
  uint64_t actual_funds;
  std::vector<libbitcoin::chain::output_point> points;
};

class PurseAccessor {
public:
  static AddressFunds obtain_funds(LibbClient &libb_client,
                                   uint64_t requested_funds,
                                   std::string address);
  static AddressFunds look_for_funds(LibbClient &libb_client,
                                     uint64_t requested_funds,
                                     std::vector<std::string> &addresses);
  static AddressFunds
  look_for_funds_by_balance(ElectrumApiClient &electrum_client,
                            LibbClient &libb_client, uint64_t requested_funds,
                            std::vector<std::string> &addresses);
};

#endif
