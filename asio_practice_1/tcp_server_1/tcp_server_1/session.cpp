//
//  session.cpp
//  tcp_server_1
//
//  Copyright © 2019 larry-kof. All rights reserved.
//

#include "session.hpp"
#include <boost/smart_ptr.hpp>
#include <vector>
#include <iostream>

extern void fail(const boost::system::error_code ec, const char* message);

session::session(tcp::socket&& socket)
:strand_(socket.get_executor())
,socket_(std::move(socket))
{
    
}

void session::run()
{
    do_read();
}

void session::on_read(boost::system::error_code ec, size_t bytes, boost::shared_ptr<std::vector<char>> readBuffer)
{
    if(ec)
    {
        fail(ec, "on read");
    }
    else
    {
        buffer_.append( &*(readBuffer->begin()) , bytes);
        while( buffer_.readableBytes() >= Buffer::kCheapPrepend )
        {
            
        }
    }
    do_read();
}

void session::do_read()
{
    auto readBuffer = boost::make_shared<std::vector<char>>(100);
    socket_.async_read_some( net::buffer(*readBuffer) ,  net::bind_executor(strand_,
                                                             std::bind(
                                                                       &session::on_read, this, std::placeholders::_1, std::placeholders::_2, readBuffer)) );
}
