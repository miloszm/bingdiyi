/**
 * Copyright (c) 2020-2021 bingdiyi developers (see AUTHORS)
 *
 * This file is part of bingdiyi.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "src/common/bing_common.hpp"
#include <bitcoin/system.hpp>
#include <boost/program_options.hpp>
#include <binglib/address_converter.hpp>


using namespace boost::program_options;
using namespace std;
using namespace bc;
using namespace bc::chain;
using namespace bc::wallet;
using namespace bc::machine;

int main(int argc, char* argv[]){
    try {
        string addr_base58;

        options_description desc("Converts address to scripthash as used by EPS/EPSMI/Electrum Server address history key.\n\nRequired options");
        desc.add_options()
                ("help,h", "print usage message")
                ("addr", value<string>(&addr_base58)->required(), "source address");

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);

        if (vm.count("help") || argc <= 1) {
            cout << "\n\n" << desc << "\n";
            cout << "example:" << "\n";
            cout << "--a=1FpeH5RojTMpaUS8oreYBRtMpCk1mfVxcf" << "\n\n";
            return 1;
        }

        // note: must be after help option check
        notify(vm);

        string converted = AddressConverter::base58_to_spkh_hex(addr_base58);
        cout << addr_base58 << " ==> " << converted << "\n";
        return 0;
    }
    catch(exception& e) {
        cerr << e.what() << "\n";
    }
}
