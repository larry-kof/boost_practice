
#include "rpc_client.hpp"

void fail(const boost::system::error_code &ec, std::string message)
{
    std::cout << message << " : " << ec.message() << std::endl;
}

rpc_client::rpc_client(net::io_context& ioc, const std::string& ip, int port)
:socket_(ioc)
,stub_(this)
,endpoint_( net::ip::make_address_v4(ip), port )
,id_(0)
{

}

void rpc_client::run()
{
    socket_.async_connect(endpoint_, std::bind( &rpc_client::on_connect, shared_from_this(), std::placeholders::_1));
}

void rpc_client::on_connect(const boost::system::error_code& ec)
{
    if(ec)
    {
        fail(ec, "on connect");
        return;
    }
    do_read();
}

void rpc_client::do_read()
{
    auto readBuffer = std::make_shared< std::vector<char> >(100);
    socket_.async_read_some( net::buffer( *readBuffer ), std::bind( &rpc_client::on_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2, readBuffer ) );
}

void rpc_client::on_read(const boost::system::error_code& ec, size_t bytes, const std::shared_ptr< std::vector<char> >& readBuffer)
{
    if( ec )
    {
        fail(ec, "on read");
        socket_.shutdown(net::socket_base::shutdown_send);
        return;
    }
    inputBuffer_.ensureWritableBytes(bytes);
    inputBuffer_.append( &*(readBuffer->begin()), bytes );
    while( inputBuffer_.readableBytes() >= sizeof(int32_t) )
    {
        int32_t len = inputBuffer_.peekInt32();
        if( inputBuffer_.readableBytes() >= sizeof(int32_t) + len )
        {
            inputBuffer_.retrieve(sizeof(int32_t));
            const rpc::test::TestRes& res_default = rpc::test::TestRes::default_instance(); 
            ResponseCall callback = {NULL, NULL};
            std::unique_ptr<rpc::test::TestRes> resTemp(res_default.New());
            resTemp->ParseFromArray(inputBuffer_.peek(), inputBuffer_.readableBytes());
            std::lock_guard<std::mutex> lck(mutex_);
            auto it = resCalls_.find(resTemp->id());
            if(it != resCalls_.end())
            {
                callback.response = it->second.response;
                callback.done = it->second.done;
                resCalls_.erase(it);
            }
            callback.response->ParseFromArray(inputBuffer_.peek(), inputBuffer_.readableBytes());
            inputBuffer_.retrieve(len);
            callback.done->Run();
        }else
        {
            break;
        }
        
    }
    do_read();
}

void rpc_client::CallMethod(const MethodDescriptor *method,
                            RpcController *controller, const Message *request,
                            Message *response, Closure *done)
{
    std::lock_guard<std::mutex> lck(mutex_);
    int32_t len = request->ByteSize();
    outputBuffer_.ensureWritableBytes(len);
    uint8_t* start = reinterpret_cast<uint8_t*>(outputBuffer_.beginWrite());
    uint8_t* end = request->SerializeWithCachedSizesToArray(start);
    if( end - start != len )
    {
        std::cout << "error call method\n"; 
    }
    outputBuffer_.hasWritten(len);
    outputBuffer_.prependInt32(len);

    resCalls_[static_cast<const rpc::test::TestReq*>(request)->id()] = { response, done };

    socket_.async_write_some( net::buffer( outputBuffer_.peek(), outputBuffer_.readableBytes() ),
        std::bind( &rpc_client::on_send, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
}

void rpc_client::on_send(const boost::system::error_code& ec, size_t bytes)
{
    if(ec)
    {
        fail(ec, "on send");
        socket_.shutdown(net::socket_base::shutdown_send);
        return;
    }
    std::lock_guard<std::mutex> lck(mutex_);
    outputBuffer_.retrieve(bytes);
    if( outputBuffer_.readableBytes() > 0 )
    {
        socket_.async_write_some( net::buffer( outputBuffer_.peek(), outputBuffer_.readableBytes() ),
            std::bind( &rpc_client::on_send, this, std::placeholders::_1, std::placeholders::_2 ) );
    }
}

void rpc_client::send_request(const std::string& msg)
{
    rpc::test::TestReq req;
    req.set_message(msg);
    req.set_id(id_);
    id_.fetch_add(1);
    const rpc::test::TestRes& res_default = rpc::test::TestRes::default_instance();
    rpc::test::TestRes* response = res_default.New();
    stub_.TestSolve(NULL, &req, response, NewCallback(this, &rpc_client::on_solved, response));
}

void rpc_client::on_solved(rpc::test::TestRes* response)
{
    std::unique_ptr<rpc::test::TestRes> d(response);
    std::cout << response->DebugString() << std::endl;
}

