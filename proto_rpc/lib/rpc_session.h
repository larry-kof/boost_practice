#ifndef RPC_SESSION_H
#define RPC_SESSION_H

#include <boost/noncopyable.hpp>
#include <memory>
#include "common/net.hpp"
#include "common/Buffer.h"
#include "rpc_codec.h"
#include <google/protobuf/service.h>
#include <google/protobuf/message.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>

namespace bean
{
namespace net
{
class rpc_session;
typedef std::function<void(const std::shared_ptr<rpc_session> &)> SessionOverFunc;
class rpc_session : private boost::noncopyable, public std::enable_shared_from_this<rpc_session>, public google::protobuf::RpcChannel
{
public:
    rpc_session(tcp::socket &&socket, SessionOverFunc sessionOver);
    void run();
    void disconnect();
    void set_services(const std::map<std::string, google::protobuf::Service *> *services);

    virtual void CallMethod(const google::protobuf::MethodDescriptor *method,
                            google::protobuf::RpcController *controller,
                            const google::protobuf::Message *request,
                            google::protobuf::Message *response,
                            google::protobuf::Closure *done);

private:
    void do_read();
    void on_read(const boost::system::error_code &ec, size_t bytes, const ReadBufferPtr &readBuffer);
    void on_rpc_callback(const RpcMessagePtr &rpcMsg, rpc_codec::CodecError error);
    void done_callback(google::protobuf::Message *response, int32_t id);

    tcp::socket socket_;
    SessionOverFunc sessionOverFunc_;
    boost_net::strand<boost_net::io_context::executor_type> strand_;
    rpc_codec codec_;
    const std::map<std::string, google::protobuf::Service *> *services_;
    std::atomic<int> id_;
    std::map<int, std::tuple<google::protobuf::Message *, google::protobuf::Closure *>> reqCallbacks_;
    std::mutex mutex_;

    friend class rpc_codec;
};

} // namespace net
} // namespace bean

#endif