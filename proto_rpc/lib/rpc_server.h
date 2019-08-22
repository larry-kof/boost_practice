
#ifndef RPC_SERVER
#define RPC_SERVER

#include <memory>
#include <vector>
#include <boost/noncopyable.hpp>
#include <google/protobuf/service.h>
#include <google/protobuf/message.h>
#include <map>
#include <string>
#include <thread>
#include <mutex>
#include "common/net.hpp"
#include "rpc_session.h"

namespace bean
{
namespace net
{
class rpc_server : private boost::noncopyable
{
public:
    explicit rpc_server(int32_t port);

    void set_thread_num(uint32_t threadNum);

    void run();
    void stop();

    void registerService(google::protobuf::Service *service);

private:
    void do_accept();
    void on_accept(const boost::system::error_code &ec);
    void on_session_over(const std::shared_ptr<rpc_session> &self);

    boost_net::io_context ioc_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    uint32_t threadNum_;

    std::map<std::string, google::protobuf::Service *> services_;
    std::vector<std::thread> threads_;

    std::vector<std::shared_ptr<rpc_session>> sessions_;

    std::mutex mutex_;
};

} // namespace net
} // namespace bean

#endif