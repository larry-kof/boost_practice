//
//  http_session.hpp
//  boost_practice
//
//  Created by larry-kof on 2019/7/28.
//  Copyright © 2019 larry-kof. All rights reserved.
//

#ifndef http_session_hpp
#define http_session_hpp

#include "net.hpp"
#include "beast.hpp"
#include <boost/smart_ptr.hpp>
#include <boost/optional.hpp>
#include <string>
#include <cstdlib>
#include <iostream>

class http_session : public boost::enable_shared_from_this<http_session>
{
private:
    tcp::socket socket_;
    std::string root_doc_;
    beast::flat_buffer buffer_;

    net::strand<net::io_context::executor_type> strand_;
    
    boost::optional<http::request_parser<http::string_body>> parser_;
    void do_read();
    
    void on_read(beast::error_code ec, size_t bytes);
    
    void on_write(beast::error_code ec, size_t bytes, bool close);
    struct send_lamda;
public:
    explicit
    http_session( tcp::socket&& socket, std::string root_doc );
    void run();
};

#endif /* http_session_hpp */
