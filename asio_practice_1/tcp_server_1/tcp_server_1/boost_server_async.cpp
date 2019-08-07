//
//  boost_server_async.cpp
//  tcp_server_1
//
//  Created by larry-kof on 2019/8/4.
//  Copyright © 2019 larry-kof. All rights reserved.
//

#include "boost_server_async.hpp"

void fail(const boost::system::error_code ec, const char* message)
{
    std::cout << message << ":" << ec.message() << std::endl;
}

boost_server::boost_server(net::io_context& ioc, tcp::endpoint endpoint)
:socket_(ioc), strand_(ioc.get_executor())
,acceptor_(ioc, endpoint)
{
    boost::system::error_code ec;
    acceptor_.open(endpoint.protocol(), ec);
    if(ec)
    {
        fail(ec, "fail to open");
        return;
    }
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if(ec)
    {
        fail(ec, "fail to set option");
        return;
    }
    
    acceptor_.bind(endpoint, ec);
    if(ec)
    {
        fail(ec, "bind");
        return;
    }
    
    acceptor_.listen(net::socket_base::max_connections, ec);
    if(ec)
    {
        fail(ec, "listen");
        return;
    }
    
}

void boost_server::start()
{
    acceptor_.async_accept(socket_, net::bind_executor(strand_, std::bind( &boost_server::on_accept, this, std::placeholders::_1)));
}

void boost_server::on_accept(boost::system::error_code ec)
{
    
}
