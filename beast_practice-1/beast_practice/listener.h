//
//  listener.hpp
//  boost_practice
//
//  Created by larry-kof on 2019/7/27.
//  Copyright © 2019 larry-kof. All rights reserved.
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
