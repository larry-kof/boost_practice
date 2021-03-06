#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H

#include "common/net.hpp"
#include <memory>
#include <boost/noncopyable.hpp>
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <string>
#include "rpc_session.h"
#ifdef USE_SSL
#include <boost/asio/ssl.hpp>
#endif

namespace bean
{
namespace net
{
template <typename CustomService_Stub>
class rpc_client : private boost::noncopyable
{
public:
    typedef std::function<void(CustomService_Stub *)> ConnectionCb;
    rpc_client(const std::string &address, int port, ConnectionCb connecionCb)
#ifdef USE_SSL
        : ioc_(), context_(boost::asio::ssl::context::sslv23), socket_(ioc_), timeOut_(ioc_), endpoint_(boost_net::ip::make_address_v4(address), port), connectionCb_(std::move(connecionCb))
#else
        : ioc_(), socket_(ioc_), timeOut_(ioc_), endpoint_(boost_net::ip::make_address_v4(address), port), connectionCb_(std::move(connecionCb))
#endif
    {
        #ifdef USE_SSL
        context_.load_verify_file("ssl_file/server.crt");
        #endif
    }
    void connect()
    {
        socket_.async_connect(endpoint_, std::bind(&rpc_client::on_connect, this, std::placeholders::_1));
        timeOut_.expires_after(std::chrono::seconds(2));
        timeOut_.async_wait([this](const boost::system::error_code& ec){
            if(!ec)
            {
                std::cout << "time out " << std::endl;
                ioc_.stop();
            }
        });
    }
    void run()
    {
        ioc_.run();
    }

    void disconnect()
    {
        session_->disconnect();
    }

private:
    void on_connect(const boost::system::error_code &ec)
    {
        timeOut_.cancel();
        if (ec)
        {
            fail(ec, "connect");
            return;
        }
        #ifdef USE_SSL
        session_.reset(new rpc_session(std::move(socket_), context_,std::bind(&rpc_client::on_session_over, this, std::placeholders::_1), [this](bool succeed){
            if(succeed)
                connectionCb_(stub_.get());
        }));
        #else
        session_.reset(new rpc_session(std::move(socket_), std::bind(&rpc_client::on_session_over, this, std::placeholders::_1)));
        #endif
        stub_.reset(new CustomService_Stub(session_.get()));
        session_->run();
        #ifndef USE_SSL
        connectionCb_(stub_.get());
        #endif
    }

    void on_session_over(const std::shared_ptr<rpc_session> &session)
    {
        if (session_ == session)
        {
            ioc_.stop();
        }
    }

    boost_net::io_context ioc_;
#ifdef USE_SSL
    boost::asio::ssl::context context_;
#endif
    tcp::socket socket_;
    std::unique_ptr<CustomService_Stub> stub_;
    std::shared_ptr<rpc_session> session_;
    tcp::endpoint endpoint_;
    ConnectionCb connectionCb_;
    boost::asio::steady_timer timeOut_;
};
} // namespace net
} // namespace bean

#endif