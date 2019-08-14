#ifndef RPC_SESSION_HPP
#define RPC_SESSION_HPP

#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <vector>

#include "../common/Buffer.h"
#include "../common/net.hpp"
#include "../common/test.pb.h"

class TestServiceImpl : public rpc::test::TestService
{
  public:
    virtual void
    TestSolve(::google::protobuf::RpcController *controller,
              const ::rpc::test::TestReq *request,
              ::rpc::test::TestRes *response,
              ::google::protobuf::Closure *done)
    {
        std::cout << "server read req id = "<<request->id() << " msg = " << request->message() << std::endl;
        response->set_id(request->id());
        std::string msg = request->message();
        for (int i = 0; i < msg.length(); i++)
        {
            msg[i] = msg[i] + 1;
        }
        response->set_message(msg);
        std::cout << "server send msg = " << response->message() << std::endl;
        if (done)
        {
            done->Run();
        }
    }
};

class rpc_session : private boost::noncopyable,
                    public std::enable_shared_from_this<rpc_session>
{
  public:
    using SessionEraseFunc = std::function<void(const std::shared_ptr<rpc_session>& self)>;
  private:
    tcp::socket socket_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;
    SessionEraseFunc sessionCb_;
    std::unique_ptr<google::protobuf::Service> service_;

    void do_read();
    void
    on_read(const boost::system::error_code &ec, size_t bytes,
            const std::shared_ptr<std::vector<char>> &readBuffer);

    const int kMinMessageLen = sizeof( int32_t );

    void doneCallback(google::protobuf::Message* response);

    void on_write(const boost::system::error_code& ec, size_t bytes);

  public:
    explicit rpc_session(tcp::socket &&socket, SessionEraseFunc sessionCb);
    void run();
};

#endif
