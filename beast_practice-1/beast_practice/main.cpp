//
//  main.cpp
//  boost_practice
//
//  Created by larry-kof on 2019/7/27.
//  Copyright © 2019 larry-kof. All rights reserved.
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
