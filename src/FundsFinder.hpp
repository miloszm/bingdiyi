#ifndef FUNDS_FINDER_HPP
#define FUNDS_FINDER_HPP

#include <bitcoin/system.hpp>

class FundsFinder {
public:
    /**
     * Creates list of utxos to provide funds needed
     * @param satoshisNeeded
     * @param points
     * @return pair of: 1) list of utxos 2) gathered funds
     * if 1) empty means funds were not sufficient, in such case 2) contains available funds
     */
    static std::pair<std::vector<libbitcoin::system::chain::output_point>, uint64_t> find_funds(uint64_t satoshisNeeded, libbitcoin::system::chain::points_value points);
};

#endif
