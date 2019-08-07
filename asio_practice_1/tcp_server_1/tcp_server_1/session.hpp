//
//  session.hpp
//  tcp_server_1
//
//  Created by naver on 2019/8/7.
//  Copyright © 2019 larry-kof. All rights reserved.
//

#ifndef session_hpp
#define session_hpp

#include <stdio.h>
#include <memory>
#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>
#include <iostream>
#include "../../common/Buffer.h"

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class session : public boost::enable_shared_from_this<session>
{
private:
    tcp::socket socket_;
    net::strand<net::io_context::executor_type> strand_;
    Buffer buffer_;
    void on_read(boost::system::error_code ec, size_t bytes, boost::shared_ptr<std::vector<char>> readBuffer);
    void do_read();
public:
    explicit session(tcp::socket &&socket);
    void run();
};

#endif /* session_hpp */
