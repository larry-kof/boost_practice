//
//  main.cpp
//  tcp_client1
//
//  Created by larry-kof on 2019/8/4.
//  Copyright © 2019 larry-kof. All rights reserved.
//

#include <boost/smart_ptr.hpp>
#include <iostream>
#include <string>
#include <thread>
#include "boost_client_async.hpp"

int main( int argc, const char *argv[] ) {
    // insert code here...

    std::string ip = argv[1];
    int port = std::atoi( argv[2] );

    tcp::endpoint point( net::ip::make_address( ip ),
                         port );
    net::io_context ioc;
    auto client = boost::make_shared< boost_client_async >(
        ioc, point );
    client->start();

    std::thread t( [&ioc]() { ioc.run(); } );

    std::string line;
    while( std::getline( std::cin, line ) ) {
        if( line == "0" ) {
            ioc.stop();
            break;
        } else if( line != "" ) {
            client->sendMessage( line );
        }
    }
    t.join();

    return 0;
}
