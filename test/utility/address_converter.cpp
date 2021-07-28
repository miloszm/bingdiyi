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
