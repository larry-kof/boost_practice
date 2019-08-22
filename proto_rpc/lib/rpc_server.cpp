
#include "rpc_server.h"
#include <memory>

void fail(const boost::system::error_code &ec, const char *msg)
{
    std::cout << msg << ":" << ec.message() << " error value = " << ec.value() << std::endl;
}

using namespace bean::net;
rpc_server::rpc_server(int32_t port)
    : ioc_(), socket_(ioc_), acceptor_(ioc_), threadNum_(1)
{
    boost::system::error_code ec;
    tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
    acceptor_.open(endpoint.protocol(), ec);
    if (ec)
    {
        fail(ec, "open");
        return;
    }

    acceptor_.set_option(boost_net::socket_base::reuse_address(true), ec);
    if (ec)
    {
        fail(ec, "set option");
        return;
    }

    acceptor_.bind(endpoint, ec);
    if (ec)
    {
        fail(ec, "bind");
        return;
    }
    acceptor_.listen(boost_net::socket_base::max_listen_connections, ec);
    if (ec)
    {
        fail(ec, "listen");
        return;
    }
}

void rpc_server::set_thread_num(uint32_t threadNum)
{
    threadNum_ = threadNum;
}

void rpc_server::run()
{
    do_accept();
    if (threadNum_ > 1)
    {
        threads_.reserve(threadNum_ - 1);
        boost_net::io_context &ioc = ioc_;
        for (int i = 0; i < threadNum_ - 1; i++)
        {
            threads_.emplace_back([&ioc]() {
                ioc.run();
            });
        }
    }
    ioc_.run();
}

void rpc_server::stop()
{
    ioc_.stop();

    if (threadNum_ > 1)
    {
        for (auto it = threads_.begin(); it != threads_.end(); it++)
        {
            (*it).join();
        }
    }
}

void rpc_server::do_accept()
{
    acceptor_.async_accept(socket_, std::bind(&rpc_server::on_accept, this, std::placeholders::_1));
}

void rpc_server::registerService(google::protobuf::Service *service)
{
    std::string fullName = service->GetDescriptor()->full_name();
    std::lock_guard<std::mutex> lck(mutex_);
    services_[fullName] = service;
}

void rpc_server::on_accept(const boost::system::error_code &ec)
{
    if (ec)
    {
        fail(ec, "on accept");
    }
    else
    {
        std::cout << "connected from " << socket_.remote_endpoint().address() << " : " << socket_.remote_endpoint().port() << std::endl;
        auto session = std::make_shared<rpc_session>(std::move(socket_), std::bind(&rpc_server::on_session_over, this, std::placeholders::_1));
        session->set_services(&services_);
        session->run();
        {
            std::lock_guard<std::mutex> lck(mutex_);
            sessions_.push_back(session);
        }
    }
    do_accept();
}

void rpc_server::on_session_over(const std::shared_ptr<rpc_session> &self)
{
    auto it = std::find(sessions_.begin(), sessions_.end(), self);
    if (it != sessions_.end())
    {
        sessions_.erase(it);
    }
}