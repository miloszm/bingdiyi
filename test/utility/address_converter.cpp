#define BOOST_TEST_MODULE bing_test
#include <boost/test/included/unit_test.hpp>

#include "utility/address_converter.hpp"
#include <iostream>

using namespace std;


BOOST_AUTO_TEST_CASE(address_conversion_test)
{
    string spk_hex = AddressConverter::base58_to_spk_hex("mpS14bFCZiHFRxfNNbnPT2FScJBrm96iLE");
    BOOST_TEST(spk_hex == "76a91461c95cddadf465cac9b0751edad16624d01572c088ac");
}
