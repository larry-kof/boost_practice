#ifndef rpc_client_hpp
#define rpc_client_hpp

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <google/protobuf/service.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <map>
#include "../common/net.hpp"
#include "test.pb.h"
#include "../common/Buffer.h"

using namespace google::protobuf;
class rpc_client : public google::protobuf::RpcChannel, private boost::noncopyable, public std::enable_shared_from_this<rpc_client> {
public:
    virtual void CallMethod(const MethodDescriptor *method,
                            RpcController *controller, const Message *request,
                            Message *response, Closure *done);
    
    explicit
    rpc_client(net::io_context& ioc, const std::string& ip, int port);
    void run();
    void send_request(const std::string& msg);
private:
    void do_read();
    void on_read(const boost::system::error_code& ec, size_t bytes, const std::shared_ptr< std::vector<char> >& readBuffer);
    void on_connect(const boost::system::error_code& ec);
    void on_send(const boost::system::error_code& ec, size_t bytes);
    void on_solved(rpc::test::TestRes* response);
    rpc::test::TestService::Stub stub_;
    tcp::socket socket_;
    tcp::endpoint endpoint_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    struct ResponseCall 
    {
        google::protobuf::Message* response;
        google::protobuf::Closure* done;
    };
    std::map<int32_t, ResponseCall> resCalls_;
    std::mutex mutex_;
    std::atomic<int> id_;
};

#endif