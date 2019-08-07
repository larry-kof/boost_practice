//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef listener_h
#define listener_h

#include "net.hpp"
#include "beast.hpp"
#include <boost/smart_ptr.hpp>

class listener : public boost::enable_shared_from_this<listener>
{
private:
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    net::strand<net::io_context::executor_type> strand_;
    
    void fail( beast::error_code ec, const char* what );
    void on_accept(beast::error_code ec);
public:
    explicit listener(net::io_context& ioc, tcp::endpoint endpoint);
    void run();
    
};

#endif /* listener_h */
