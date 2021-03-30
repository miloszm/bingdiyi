#include "purse_accessor.hpp"
#include "funds_finder.hpp"
#include <binglib/address_converter.hpp>

using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;

AddressFunds PurseAccessor::obtain_funds(LibbClient &libb_client,
                                         uint64_t requested_funds,
                                         string address) {
  auto points_value =
      libb_client.fetch_utxo(payment_address(address), requested_funds,
                             wallet::select_outputs::algorithm::individual);
  cout << "checking " << address << "\n";
  auto utxos_funds = FundsFinder::find_funds(requested_funds, points_value);
  return AddressFunds{address, requested_funds, utxos_funds.second,
                      utxos_funds.first};
}

AddressFunds PurseAccessor::look_for_funds(LibbClient &libb_client,
                                           uint64_t requested_funds,
                                           vector<string> addresses) {
  AddressFunds maxFunds = AddressFunds{"", 0, 0, vector<output_point>()};
  for (auto a : addresses) {
    auto funds = obtain_funds(libb_client, requested_funds, a);
    if (funds.actual_funds >= requested_funds)
      return funds;
    if (funds.actual_funds >= maxFunds.actual_funds)
      maxFunds = funds;
  }
  return maxFunds;
}

AddressFunds PurseAccessor::look_for_funds_by_balance(
    ElectrumApiClient &electrum_api_client, LibbClient &libb_client,
    uint64_t requested_funds, std::vector<std::string> addresses) {
    AddressFunds maxFunds = AddressFunds{"", 0, 0, vector<output_point>()};
    for (auto a : addresses) {
        cout << "checking " << a << "\n";
        string address_spkh = AddressConverter::base58_to_spkh_hex(a);
        AddressBalance balance = electrum_api_client.getBalance(address_spkh);
        if (balance.confirmed >= requested_funds){
            auto funds = obtain_funds(libb_client, requested_funds, a);
            if (funds.actual_funds >= requested_funds)
                return funds;
            if (funds.actual_funds >= maxFunds.actual_funds)
                maxFunds = funds;
        }
        else {
            if (balance.confirmed >= maxFunds.actual_funds) {
                uint64_t balance_actual = balance.confirmed;
                maxFunds = AddressFunds{a, requested_funds, balance_actual, vector<output_point>()};
            }
        }
    }
    return maxFunds;
}
