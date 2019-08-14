
#include "boost_async.hpp"
#include <memory>
#include <thread>
#include <vector>

int main(int argc, char* argv[])
{
    if(argc < 2) 
    {
        printf("missing necessary parameter");
        exit(1);
    }
    int port = std::atoi(argv[1]);
    boost::asio::ip::tcp::endpoint point( boost::asio::ip::tcp::v4(), port );
    int threads = argc >= 3? std::atoi(argv[2]) : 1;
    boost::asio::io_context ioc;
    auto server = std::make_shared<boost_async>(ioc, point);
    server->run();

    std::vector<std::thread> v(threads-1);
    for( int i = 0; i < v.size(); i++ )
    {
        v.emplace_back( [&ioc]()
        {
            ioc.run();
        } );
    }
    ioc.run();
    return 0;
}