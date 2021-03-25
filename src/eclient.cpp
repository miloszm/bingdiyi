//
// client.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/asio/ssl.hpp>
#include <nlohmann/json.hpp>

using boost::asio::ip::tcp;
using std::placeholders::_1;
using std::placeholders::_2;
using json = nlohmann::json;
using namespace std;

enum { max_length = 1024 };


/**
 * ======================================================
procedure to convert address to what get_history needs:

say address is: mpS14bFCZiHFRxfNNbnPT2FScJBrm96iLE

do:

bitcoin-cli getaddressinfo mpS14bFCZiHFRxfNNbnPT2FScJBrm96iLE

read scriptPubKey from the output of ^^:
"scriptPubKey": "76a91461c95cddadf465cac9b0751edad16624d01572c088ac"

then do:

bx sha256 76a91461c95cddadf465cac9b0751edad16624d01572c088ac

this produces: 04f0d935b98f356c0c87bd23b51be014ec6ad60038222be09edf5d9188af89af

now you need to reverse it byte-wise:
af89af88915ddf9ee02b223800d66aec14e01bb523bd870c6c358fb935d9f004

this is it ^^
to sum up: it is a reversed sha256 of scriptPubKey of the address
========================================================

bx base58-decode mpS14bFCZiHFRxfNNbnPT2FScJBrm96iLE
6f61c95cddadf465cac9b0751edad16624d01572c066ff8027

first byte is sth else, last 4 bytes are checksum in little endian
the rest needs to have 76a914 prepended and 88ac appended
this is it
then just sha256, then byte-wise reverse and we are done

========================================================
 */

class client
{
public:
    client(boost::asio::io_context& io_context,
           boost::asio::ssl::context& context,
           const tcp::resolver::results_type& endpoints)
            : socket_(io_context, context)
    {
        prepare_connection.lock();
        socket_.set_verify_mode(boost::asio::ssl::verify_none);
        socket_.set_verify_callback(
                std::bind(&client::verify_certificate, this, _1, _2));

        connect(endpoints);
    }
    std::mutex prepare_connection;

private:
    bool verify_certificate(bool preverified,
                            boost::asio::ssl::verify_context& ctx)
    {
        // The verify callback can be used to check whether the certificate that is
        // being presented is valid for the peer. For example, RFC 2818 describes
        // the steps involved in doing this for HTTPS. Consult the OpenSSL
        // documentation for more details. Note that the callback is called once
        // for each certificate in the certificate chain, starting from the root
        // certificate authority.

        // In this example we will simply print the certificate's subject name.
        char subject_name[256];
        X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
        X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
        std::cout << "Verifying: " << subject_name << "\n";

        return preverified;
    }

    void connect(const tcp::resolver::results_type& endpoints)
    {
        boost::asio::async_connect(socket_.lowest_layer(), endpoints,
                                   [this](const boost::system::error_code& error,
                                          const tcp::endpoint& /*endpoint*/)
                                   {
                                       if (!error)
                                       {
                                           handshake();
                                       }
                                       else
                                       {
                                           std::cout << "Connect failed: " << error.message() << "\n";
                                       }
                                   });
    }

    void handshake()
    {
        socket_.async_handshake(boost::asio::ssl::stream_base::client,
                                [this](const boost::system::error_code& error)
                                {
                                    if (!error)
                                    {
                                        std::cout << "unlocking prepare_connection" << "\n";
                                        prepare_connection.unlock();
                                    }
                                    else
                                    {
                                        std::cout << "Handshake failed: " << error.message() << "\n";
                                    }

                                });
    }

public:
    void send_request(json json_request)
    {
        std::string req0 = json_request.dump();
        std::string req = req0 + "\n";
        strcpy(request_, req.data());
        size_t request_length = std::strlen(req.data());

        boost::asio::write(socket_,boost::asio::buffer(request_, request_length));
    }

    json receive_response()
    {
        std::ostringstream oss;
        for (;;)
        {
            boost::array<char, 512> buf;
            boost::system::error_code error;

            size_t len = socket_.read_some(boost::asio::buffer(buf), error);

            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error); // Some other error.

            oss.write(buf.data(), len);
            std::string candidate_response = oss.str();
            try {
                json parsed_response = json::parse(candidate_response);
                return parsed_response;
            } catch (json::parse_error& e){
                // not yet parsable, keep reading
                continue;
            }
        }
        return json::parse("{}");
    }

private:
    boost::asio::ssl::stream<tcp::socket> socket_;
    char request_[max_length];
    char reply_[max_length];
};

int main(int argc, char* argv[])
{
    try
    {
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("localhost", "51002");
//        auto endpoints = resolver.resolve("blockstream.info", "993");
//        auto endpoints = resolver.resolve("testnet.qtornado.com", "51002");
//        auto endpoints = resolver.resolve("testnet.electrumx.hodlwallet.com", "51002");

        boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
        ctx.load_verify_file("cert.crt");

        client c(io_context, ctx, endpoints);

        io_context.run();

        std::cout << "waiting for connection..." << "\n";

        c.prepare_connection.lock();

        std::cout << "sending request..." << "\n";

        std::string x = "04f0d935b98f356c0c87bd23b51be014ec6ad60038222be09edf5d9188af89af";
        std::cout << "reversed=\n";
        for (int i = 31; i >= 0; --i)
            std::cout << x.at(i*2) << x.at(i*2+1);
        std::cout << "\n";

        json banner_request = R"({"jsonrpc":"2.0","method":"server.banner","id":1712})"_json;
        json get_history_request = R"({"jsonrpc":"2.0","method":"blockchain.scripthash.get_history","id":1712,"params":["af89af88915ddf9ee02b223800d66aec14e01bb523bd870c6c358fb935d9f004"]})"_json;

        c.send_request(get_history_request);
        cout << "response = \n";
        json j = c.receive_response();
        cout << j.dump(4) << "\n";
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
