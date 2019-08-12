#include "boost_async.hpp"
#include <iostream>

void fail(const boost::system::error_code &ec, std::string message)
{
    std::cout << message << " : " << ec.message() << std::endl;
}

boost_async::boost_async(net::io_context &ioc, tcp::endpoint point)
    : ioc_(ioc), socket_(ioc), acceptor_(ioc)
{
    boost::system::error_code ec;
    acceptor_.open(point.protocol(), ec);
    if (ec)
    {
        fail(ec, "open");
        return;
    }

    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec)
    {
        fail(ec, "set option");
        return;
    }

    acceptor_.bind(point, ec);
    if (ec)
    {
        fail(ec, "bind");
        return;
    }

    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec)
    {
        fail(ec, "listen");
        return;
    }
}

void boost_async::run()
{
    do_accept();
}

void boost_async::do_accept()
{
    acceptor_.async_accept(socket_, std::bind(&boost_async::on_accept, this, std::placeholders::_1));
}

void boost_async::on_accept(const boost::system::error_code &ec)
{
    if(ec)
    {
        fail(ec, "on_accept");
        socket_.shutdown(net::socket_base::shutdown_both);
        return;
    }
    
    do_accept();
}