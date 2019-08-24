#include "../lib/rpc_server.h"
#include "test.pb.h"
#include <string>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

using namespace bean::net;
class TestServerImpl : public rpc::test::TestService
{
public:
    virtual void Add(::google::protobuf::RpcController *controller,
                     const ::rpc::test::TestReq *request,
                     ::rpc::test::TestRes *response,
                     ::google::protobuf::Closure *done)
    {
        int result = 0;
        int size = request->value_size();
        printf("size = %d \n", size);
        for (int i = 0; i < size; i++)
        {
            printf("value = %d \n", request->value(i));
            result += request->value(i);
        }
        response->set_result(result);
        done->Run();
    }
};

int main(int argc, char **argv)
{
    int port = std::atoi(argv[1]);
    auto server = std::make_shared<rpc_server>(port);
    TestServerImpl testService;
    server->registerService(&testService);
    server->set_thread_num(4);
    server->run();
    return 0;
}