//
//  boost_client_async.cpp
//  tcp_client1
//
//  Copyright © 2019 larry-kof. All rights reserved.
//

#include "boost_client_async.hpp"

void fail( const boost::system::error_code ec,
           const char *message ) {
    std::cout << message << ":" << ec.message()
              << std::endl;
}

boost_client_async::boost_client_async(
    net::io_context &ioc, tcp::endpoint point )
    : socket_( ioc ), strand_( ioc.get_executor() ),
      endpoint_( point ) {}

void boost_client_async::start() {
    socket_.async_connect(
        endpoint_,
        std::bind( &boost_client_async::on_connected, this,
                   std::placeholders::_1 ) );
}

void boost_client_async::on_connected(
    boost::system::error_code ec ) {
    if( ec ) {
        fail( ec, "on connect" );
        return;
    }

    do_read();
}

void boost_client_async::do_read() {
    auto readBuffer =
        boost::make_shared< std::vector< char > >( 200 );
    socket_.async_read_some(
        net::buffer( *readBuffer ),
        net::bind_executor(
            strand_,
            std::bind(
                &boost_client_async::on_read,
                shared_from_this(), std::placeholders::_1,
                std::placeholders::_2, readBuffer ) ) );
}

void boost_client_async::on_read(
    boost::system::error_code ec, size_t bytes,
    const boost::shared_ptr< std::vector< char > >
        &readBuffer ) {
    if( ec ) {
        fail( ec, "on_read" );
        socket_.shutdown( tcp::socket::shutdown_both );
        return;
    }
    inputBuffer_.append( &*( readBuffer->begin() ), bytes );
    while( inputBuffer_.readableBytes() >
           sizeof( int32_t ) ) {
        int32_t len = inputBuffer_.peekInt32();
        if( inputBuffer_.readableBytes() >=
            len + sizeof( int32_t ) ) {
            inputBuffer_.retrieve( sizeof( int32_t ) );
            std::string message =
                inputBuffer_.retrieveAsString( len );
            std::cout << "message : " << message
                      << std::endl;
        } else {
            break;
        }
    }
    do_read();
}

void boost_client_async::on_write(
    boost::system::error_code ec, size_t bytes ) {
    if( ec ) {
        fail( ec, "on_write" );
        socket_.shutdown( tcp::socket::shutdown_both );
        return;
    }
    outputBuffer_.retrieve( bytes );
    if( outputBuffer_.readableBytes() > 0 ) {
        socket_.async_write_some(
            net::buffer( outputBuffer_.peek(),
                         outputBuffer_.readableBytes() ),
            net::bind_executor(
                strand_,
                std::bind( &boost_client_async::on_write,
                           shared_from_this(),
                           std::placeholders::_1,
                           std::placeholders::_2 ) ) );
    }
}

void boost_client_async::sendMessage(
    const std::string &message ) {
    outputBuffer_.append( message.c_str(),
                          message.length() );
    outputBuffer_.prependInt32( (int32_t)message.length() );
    socket_.async_write_some(
        net::buffer( outputBuffer_.peek(),
                     outputBuffer_.readableBytes() ),
        net::bind_executor(
            strand_,
            std::bind( &boost_client_async::on_write,
                       shared_from_this(),
                       std::placeholders::_1,
                       std::placeholders::_2 ) ) );
}
