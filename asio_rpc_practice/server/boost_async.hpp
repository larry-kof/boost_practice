#ifndef boost_async_hpp
#define boost_async_hpp

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <memory>
#include <string>
#include "../common/net.hpp"

class boost_async : private boost::noncopyable
{
private:
    net::io_context &ioc_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    void do_accept();
    void on_accept(const boost::system::error_code &ec);

public:
    explicit boost_async(net::io_context &ioc, tcp::endpoint point);
    void run();
};

#endif
