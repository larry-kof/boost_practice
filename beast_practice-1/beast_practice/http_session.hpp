//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
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
