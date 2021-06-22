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