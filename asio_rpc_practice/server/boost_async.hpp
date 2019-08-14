#ifndef boost_async_hpp
#define boost_async_hpp

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include "../common/net.hpp"
#include "rpc_session.hpp"

#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   // no-op
#endif

#define GUARDED_BY(x) THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

class boost_async : private boost::noncopyable
{
private:
    net::io_context &ioc_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::mutex mutex_;
    std::vector<std::shared_ptr<rpc_session>> sessions_  GUARDED_BY(mutex_);
    void do_accept();
    void on_accept(const boost::system::error_code &ec);

    void on_session_over(const std::shared_ptr<rpc_session>& session);

public:
    explicit boost_async(net::io_context &ioc, tcp::endpoint point);
    void run();
};

#endif
