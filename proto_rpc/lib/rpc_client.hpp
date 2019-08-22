#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H

#include "common/net.hpp"
#include <memory>
#include <boost/noncopyable.hpp>
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <string>
#include "rpc_session.h"

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
        : ioc_(), socket_(ioc_), endpoint_(boost_net::ip::make_address_v4(address), port), connectionCb_(std::move(connecionCb))
    {
    }
    void connect()
    {
        socket_.async_connect(endpoint_, std::bind(&rpc_client::on_connect, this, std::placeholders::_1));
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
        if (ec)
        {
            fail(ec, "connect");
            return;
        }
        session_.reset(new rpc_session(std::move(socket_), std::bind(&rpc_client::on_session_over, this, std::placeholders::_1)));
        stub_.reset(new CustomService_Stub(session_.get()));
        connectionCb_(stub_.get());
        session_->run();
    }

    void on_session_over(const std::shared_ptr<rpc_session> &session)
    {
        if (session_ == session)
        {
            ioc_.stop();
        }
    }

    boost_net::io_context ioc_;
    tcp::socket socket_;
    std::unique_ptr<CustomService_Stub> stub_;
    std::shared_ptr<rpc_session> session_;
    tcp::endpoint endpoint_;
    ConnectionCb connectionCb_;
};
} // namespace net
} // namespace bean

#endif