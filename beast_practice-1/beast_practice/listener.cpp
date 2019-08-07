//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#include <stdio.h>
#include "listener.h"
#include "http_session.hpp"
#include <boost/bind.hpp>

listener::listener(net::io_context& ioc, tcp::endpoint endpoint)
:ioc_(ioc), acceptor_(ioc_), socket_(ioc), strand_(ioc.get_executor())
{
    beast::error_code ec;
    acceptor_.open(endpoint.protocol(), ec);
    if(ec)
    {
        fail(ec, "open");
        return;
    }
    
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if(ec)
    {
        fail(ec, "set option");
        return;
    }
    
    acceptor_.bind(endpoint, ec);
    if(ec)
    {
        fail(ec, "bind");
        return;
    }
    
    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if(ec)
    {
        fail(ec, "listen");
        return;
    }
}

void listener::fail(beast::error_code ec, const char *what)
{
    if(ec == net::error::operation_aborted)
        return;
    std::cerr << what << ": " << ec.message() << "\n";
}

void listener::run()
{    
    acceptor_.async_accept(socket_, boost::asio::bind_executor(strand_, std::bind( &listener::on_accept, shared_from_this(), std::placeholders::_1 )));
}

void listener::on_accept(beast::error_code ec)
{
    if(ec)
    {
        fail(ec, "on accept");
    }
    else
    {
        boost::make_shared<http_session>(std::move(socket_), "/Users/apple/Documents/c++server/boost_practice/beast_practice-1/beast_practice/folder")->run();
    }
    acceptor_.async_accept(socket_, boost::asio::bind_executor(strand_, std::bind( &listener::on_accept, shared_from_this(), std::placeholders::_1 )));
}
