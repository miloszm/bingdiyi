#include "src/common/bing_common.hpp"
#include <bitcoin/bitcoin.hpp>
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

        options_description desc("Converts address to epsmi key");
        desc.add_options()
                ("help,h", "print usage message")
                ("addr", value<string>(&addr_base58)->required(), "source address");

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);

        if (vm.count("help") || argc <= 1) {
            cout << "\n\n" << desc << "\n";
            cout << "example:" << "\n";
            cout << "--a=mpS14bFCZiHFRxfNNbnPT2FScJBrm96iLE" << "\n\n";
            return 1;
        }

        // note: must be after help option check
        notify(vm);

        string converted = AddressConverter::base58_to_spkh_hex("mpS14bFCZiHFRxfNNbnPT2FScJBrm96iLE");
        cout << converted << "\n";
        return 0;
    }
    catch(exception& e) {
        cerr << e.what() << "\n";
    }
}
