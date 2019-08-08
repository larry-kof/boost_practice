//
//  session.cpp
//  tcp_server_1
//
//  Copyright © 2019 larry-kof. All rights reserved.
//

#include <boost/smart_ptr.hpp>
#include <iostream>
#include <vector>
#include "session.hpp"

extern void fail( const boost::system::error_code ec,
                  const char *message );

session::session( tcp::socket &&socket, MessageCB messageCb,
                  DisconnectCB disCb )
    : strand_( socket.get_executor() ),
      socket_( std::move( socket ) ),
      messageCb_( messageCb ), disCb_( disCb ) {}

void session::run() { do_read(); }

void session::on_read(
    boost::system::error_code ec, size_t bytes,
    const boost::shared_ptr< std::vector< char > >
        &readBuffer ) {
    if( ec ) {
        fail( ec, "on read" );
        if( disCb_ ) {
            disCb_( this );
        }
    } else {
        inputBuffer_.append( &*( readBuffer->begin() ),
                             bytes );
        while( inputBuffer_.readableBytes() >
               sizeof( int32_t ) ) {
            int32_t len = inputBuffer_.peekInt32();
            if( inputBuffer_.readableBytes() >=
                len + sizeof( int32_t ) ) {
                inputBuffer_.retrieve( sizeof( int32_t ) );
                auto messagePtr =
                    boost::make_shared< std::string >(
                        inputBuffer_.peek(), len );
                inputBuffer_.retrieve( len );
                if( messageCb_ )
                    messageCb_( messagePtr, this );
            } else {
                break;
            }
        }
        do_read();
    }
}

void session::do_read() {
    auto readBuffer =
        boost::make_shared< std::vector< char > >( 30 );
    socket_.async_read_some(
        net::buffer( *readBuffer ),
        net::bind_executor(
            strand_, std::bind( &session::on_read, this,
                                std::placeholders::_1,
                                std::placeholders::_2,
                                readBuffer ) ) );
}

void session::sendMessage(
    const boost::shared_ptr< std::string > message ) {
    outputBuffer_.append( message->c_str(),
                          message->length() );
    outputBuffer_.prependInt32(
        ( int32_t )( *message ).length() );

    socket_.async_write_some(
        net::buffer( outputBuffer_.peek(),
                     outputBuffer_.readableBytes() ),
        net::bind_executor(
            strand_, std::bind( &session::on_write,
                                shared_from_this(),
                                std::placeholders::_1,
                                std::placeholders::_2 ) ) );
}

void session::on_write( boost::system::error_code ec,
                        size_t bytes ) {
    if( ec ) {
        fail( ec, "send message" );
        if( disCb_ ) {
            disCb_( this );
        }
        return;
    }
    std::cout << bytes << " bytes has benn sent"
              << std::endl;
    outputBuffer_.retrieve( bytes );
    if( outputBuffer_.readableBytes() > 0 ) {
        socket_.async_write_some(
            net::buffer( outputBuffer_.peek(),
                         outputBuffer_.readableBytes() ),
            net::bind_executor(
                strand_,
                std::bind( &session::on_write,
                           shared_from_this(),
                           std::placeholders::_1,
                           std::placeholders::_2 ) ) );
    }
}
