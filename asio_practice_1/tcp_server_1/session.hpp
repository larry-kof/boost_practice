//
//  session.hpp
//  tcp_server_1
//
//  Created by naver on 2019/8/7.
//  Copyright © 2019 larry-kof. All rights reserved.
//

#ifndef session_hpp
#define session_hpp

#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <stdio.h>
#include "../common/Buffer.h"

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class session
    : public boost::enable_shared_from_this< session > {
    typedef std::function< void(
        boost::shared_ptr< std::string > message,
        session *self ) >
        MessageCB;
    typedef std::function< void( session *self ) >
        DisconnectCB;

private:
    tcp::socket socket_;
    net::strand< net::io_context::executor_type > strand_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    MessageCB messageCb_;
    DisconnectCB disCb_;
    void
    on_read( boost::system::error_code ec, size_t bytes,
             const boost::shared_ptr< std::vector< char > >
                 &readBuffer );
    void do_read();

    void on_write( boost::system::error_code ec,
                   size_t bytes );

public:
    session( tcp::socket &&socket, MessageCB messageCb,
             DisconnectCB disCb );
    void run();

    void sendMessage(
        const boost::shared_ptr< std::string > message );
};

inline bool
operator==( const boost::shared_ptr< session > &lhs,
            const session *rhs ) {
    return lhs.get() == rhs;
}

#endif /* session_hpp */
