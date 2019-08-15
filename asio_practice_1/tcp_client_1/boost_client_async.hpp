//
//  boost_client_async.hpp
//  tcp_client1
//
//  Copyright © 2019 larry-kof. All rights reserved.
//

#ifndef boost_client_async_hpp
#define boost_client_async_hpp

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <vector>
#include "../common/Buffer.h"

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class boost_client_async
    : private boost::noncopyable,
      public boost::enable_shared_from_this<
          boost_client_async > {
private:
    net::strand< net::io_context::executor_type > strand_;
    tcp::socket socket_;
    tcp::endpoint endpoint_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;

    void on_connected( boost::system::error_code ec );

    void do_read();
    void
    on_read( boost::system::error_code ec, size_t bytes,
             const boost::shared_ptr< std::vector< char > >
                 &readBuffer );

    void on_write( boost::system::error_code ec,
                   size_t bytes );

public:
    boost_client_async( net::io_context &ioc,
                        tcp::endpoint point );
    void start();
    void sendMessage( const std::string &message );
};

#endif /* boost_client_async_hpp */
