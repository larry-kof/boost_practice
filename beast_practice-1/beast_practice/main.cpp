//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#include <iostream>
#include "listener.h"
#include <string>
#include <vector>
#include <thread>

int main(int argc, const char * argv[]) {
    // insert code here...    
    std::string ip = argv[1];
    int port = std::stoi(argv[2]);
    int threads = std::stoi(argv[3]);
    
    std::cout<< "ip = " << ip << std::endl;
    std::cout<< "port =  "<< port << std::endl;
    std::cout<< "threads = "<< threads << std::endl;
    
    net::io_context ioc;
    auto listenObj = boost::make_shared<listener>( ioc, tcp::endpoint(net::ip::make_address(ip), port) );
    
    listenObj->run();
    std::vector<std::thread> v;
    v.reserve(threads);
    for(int i = threads - 1; i > 0; i--)
    {
        v.emplace_back([&ioc](){
            ioc.run();
        } );
    }
    
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
       [&ioc](boost::system::error_code const&, int)
       {
           // Stop the io_context. This will cause run()
           // to return immediately, eventually destroying the
           // io_context and any remaining handlers in it.
           ioc.stop();
       });
    
    ioc.run();
    
    return 0;
}
