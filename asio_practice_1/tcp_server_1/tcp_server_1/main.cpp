//
//  main.cpp
//  tcp_server_1
//
//  Created by larry-kof on 2019/8/4.
//  Copyright © 2019 larry-kof. All rights reserved.
//

#include <iostream>
#include <memory>
#include "boost_server_async.hpp"

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    int port = std::atoi(argv[1]);
    
    net::io_context ioc;
    tcp::endpoint point( net::ip::tcp::v4(), port );
    auto server = std::make_shared<boost_server>( ioc, point );
    server->start();
    
    ioc.run();
    
    return 0;
}
