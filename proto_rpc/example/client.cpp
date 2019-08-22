#include "test.pb.h"
#include "../lib/rpc_client.hpp"
#include <string>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
using namespace bean::net;

class Client
{
private:
    std::shared_ptr<rpc_client<rpc::test::TestService::Stub>> rpc_client_;

public:
    Client(const std::string &ip, int port)
        : rpc_client_(new rpc_client<rpc::test::TestService::Stub>(ip, port, std::bind(&Client::on_connection, this, std::placeholders::_1)))
    {
    }
    void done_callback(rpc::test::TestRes *response)
    {
        int result = response->result();
        std::cout << "result is " << result << std::endl;
        delete response;

        rpc_client_->disconnect();
    }

    void on_connection(rpc::test::TestService::Stub *stub)
    {
        rpc::test::TestReq req;
        req.add_value(1);
        req.add_value(2);

        rpc::test::TestRes *res = new rpc::test::TestRes();
        stub->Add(NULL, &req, res, NewCallback(this, &Client::done_callback, res));
    }
    void connect()
    {
        rpc_client_->connect();
    }

    void run()
    {
        rpc_client_->run();
    }
};

int main(int argc, char **argv)
{
    std::string ip = argv[1];
    int port = std::atoi(argv[2]);

    auto client = std::make_shared<Client>(ip, port);

    client->connect();
    client->run();
}