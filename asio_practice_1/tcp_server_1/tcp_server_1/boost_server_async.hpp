//
//  boost_server_async.hpp
//  tcp_server_1
//
//  Created by larry-kof on 2019/8/4.
//  Copyright © 2019 larry-kof. All rights reserved.
//

#ifndef boost_server_async_hpp
#define boost_server_async_hpp

#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <mutex>

class session;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class boost_server {
    typedef boost::shared_ptr< session > SessionPtr;

private:
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    net::strand< net::io_context::executor_type > strand_;
    std::vector< SessionPtr > sessions_;
    std::mutex mutex_;

    void on_accept( boost::system::error_code ec );

    void on_session_message(
        boost::shared_ptr< std::string > message,
        session *self );
    void on_session_disconnect( session *self );

public:
    boost_server( net::io_context &ioc,
                  tcp::endpoint endpoint );
    void start();
};

#endif /* boost_server_async_hpp */
