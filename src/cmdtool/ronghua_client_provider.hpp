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
#ifndef RONGHUA_CLIENT_PROVIDER_HPP
#define RONGHUA_CLIENT_PROVIDER_HPP

#include <binglib/ronghua_client.hpp>

class RonghuaClientProvider : public XElectrumInterface {
public:
    explicit RonghuaClientProvider(RonghuaClient &ronghua_client)
            : ronghua_client_(ronghua_client) {}
    virtual ~RonghuaClientProvider() {}
    ElectrumInterface &client() override { return ronghua_client_; }

private:
    RonghuaClient &ronghua_client_;
};

#endif