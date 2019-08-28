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

#ifdef USE_SSL
#include <boost/asio/ssl.hpp>
#endif

namespace bean
{
namespace net
{
class rpc_session;
typedef std::function<void(const std::shared_ptr<rpc_session> &)> SessionOverFunc;
#ifdef USE_SSL
typedef std::function<void(bool)> SSLComplete;
#endif
class rpc_session : private boost::noncopyable, public std::enable_shared_from_this<rpc_session>, public google::protobuf::RpcChannel
{
public:
#ifdef USE_SSL
    rpc_session(tcp::socket &&socket, boost_net::ssl::context& context, SessionOverFunc sessionOver, 
    SSLComplete sslComplete = NULL);
#else
    rpc_session(tcp::socket &&socket, SessionOverFunc sessionOver);
#endif
    void run(bool isServer = false);
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
    void time_out(const boost::system::error_code& ec);

#ifdef USE_SSL
    void do_handshake(bool isServer);
#endif

#ifdef USE_SSL
    boost_net::ssl::stream<tcp::socket> socket_;
#else
    tcp::socket socket_;
#endif
    SessionOverFunc sessionOverFunc_;
    SSLComplete sslComplete_;
    boost_net::strand<boost_net::io_context::executor_type> strand_;
    rpc_codec codec_;
    const std::map<std::string, google::protobuf::Service *> *services_;
    std::atomic<int> id_;
    std::map<int, std::tuple<google::protobuf::Message *, google::protobuf::Closure *>> reqCallbacks_;
    std::mutex mutex_;
    boost_net::steady_timer timeOut_;

    friend class rpc_codec;
};

} // namespace net
} // namespace bean

#endif