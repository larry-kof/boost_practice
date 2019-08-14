
#include "rpc_client.hpp"
#include <thread>
#include <string>

int main(int argc, char** argv)
{
    std::string ip = argv[1];
    int port = std::atoi(argv[2]);
    net::io_context ioc;

    auto client = std::make_shared<rpc_client>(ioc, ip, port);
    client->run();

    std::thread t([&ioc](){
        ioc.run();
    });

    std::string message;
    while( std::getline(std::cin, message) )
    {
        if(message == "0")
            break;
        client->send_request(message);
    }
    ioc.stop();
    t.join();
    return 0;
}