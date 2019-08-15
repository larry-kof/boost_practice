
//
//  main.cpp
//  tcp_server_1
//
//  Created by larry-kof on 2019/8/4.
//  Copyright © 2019 larry-kof. All rights reserved.
//

#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include "boost_server_async.hpp"

int main( int argc, const char *argv[] ) {
    // insert code here...
    std::cout << "Hello, World!\n";
    int port = std::atoi( argv[1] );
    int threads = 1;
    if( argc == 3 ) {
        threads = std::atoi( argv[2] );
        if( threads <= 1 )
            threads = 1;
    }

    std::vector< std::thread > v;
    v.reserve( threads - 1 );

    net::io_context ioc;
    net::signal_set signals( ioc, SIGINT, SIGTERM );
    signals.async_wait(
        [&]( boost::system::error_code const &, int ) {
            ioc.stop();
        } );

    tcp::endpoint point( net::ip::tcp::v4(), port );
    auto server =
        std::make_shared< boost_server >( ioc, point );
    server->start();

    for( int i = 0; i < threads - 1; i++ ) {
        v.emplace_back( [&ioc]() { ioc.run(); } );
    }
    ioc.run();

    for( auto &it : v ) {
        it.join();
    }

    return 0;
}
