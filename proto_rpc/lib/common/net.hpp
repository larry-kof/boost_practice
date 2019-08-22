#ifndef COMMON_NET_HPP
#define COMMON_NET_HPP

#include <boost/asio.hpp>
#include <iostream>
namespace boost_net = boost::asio;
using tcp = boost::asio::ip::tcp;
void fail(const boost::system::error_code &ec, const char *msg);

#endif