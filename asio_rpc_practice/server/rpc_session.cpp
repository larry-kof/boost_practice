#include "rpc_session.hpp"
#include <google/protobuf/descriptor.h>

extern void fail(const boost::system::error_code &ec,
                 std::string message);
rpc_session::rpc_session(tcp::socket &&socket, SessionEraseFunc sessionCb)
    : socket_(std::move(socket)), service_(new TestServiceImpl())
    , sessionCb_(sessionCb)
{
}

void rpc_session::run() { do_read(); }

void rpc_session::do_read()
{
    auto readBuffer = std::make_shared<std::vector<char>>(100);
    socket_.async_read_some(
        net::buffer(*readBuffer),
        std::bind(&rpc_session::on_read, shared_from_this(),
                  std::placeholders::_1, std::placeholders::_2,
                  readBuffer));
}

void rpc_session::on_read(
    const boost::system::error_code &ec, size_t bytes,
    const std::shared_ptr<std::vector<char>> &readBuffer)
{
    if (ec)
    {
        fail(ec, "on read");
        if (ec.value() == net::error::eof)
        {
            std::cout << "disconnect from " << socket_.remote_endpoint().address() << ":" << socket_.remote_endpoint().port() << std::endl;
        }
        socket_.shutdown(net::socket_base::shutdown_send);
        if(sessionCb_) sessionCb_(shared_from_this());
        return;
    }
    inputBuffer_.ensureWritableBytes(bytes);
    inputBuffer_.append(&*(readBuffer->begin()), bytes);
    while (inputBuffer_.readableBytes() >= kMinMessageLen)
    {
        int32_t len = inputBuffer_.peekInt32();

        if (inputBuffer_.readableBytes() >= len + kMinMessageLen)
        {
            inputBuffer_.retrieve(kMinMessageLen);
            const google::protobuf::MethodDescriptor *method =
                service_->GetDescriptor()->FindMethodByName(
                    "TestSolve");
            std::string msg = inputBuffer_.retrieveAsString(len);
            if (method)
            {
                std::unique_ptr<google::protobuf::Message> request(
                    service_->GetRequestPrototype(method).New());
                request->ParseFromString(msg);
                google::protobuf::Message *response =
                    service_->GetResponsePrototype(method).New();
                service_->CallMethod(
                    method, NULL, request.get(), response,
                    NewCallback(this, &rpc_session::doneCallback, response));
            }
        }
        else
        {
            break;
        }
    }
}

void rpc_session::doneCallback(google::protobuf::Message *response)
{
    std::unique_ptr<google::protobuf::Message> d(response);
    std::string message = response->SerializeAsString();

    outputBuffer_.append(message);
    int32_t len = message.length();
    outputBuffer_.prependInt32(len);

    socket_.async_write_some(
        net::buffer(net::const_buffer(outputBuffer_.peek(),
                                      outputBuffer_.readableBytes())),
        std::bind(&rpc_session::on_write, shared_from_this(),
                  std::placeholders::_1, std::placeholders::_2));
}

void rpc_session::on_write(const boost::system::error_code &ec,
                           size_t bytes)
{
    if (ec)
    {
        fail(ec, "on write");
        socket_.shutdown(net::socket_base::shutdown_send);
        if(sessionCb_) sessionCb_(shared_from_this());
        return;
    }
    outputBuffer_.retrieve(bytes);
    if (outputBuffer_.readableBytes() > 0)
    {
        socket_.async_write_some(
            net::buffer(net::const_buffer(
                outputBuffer_.peek(), outputBuffer_.readableBytes())),
            std::bind(&rpc_session::on_write, shared_from_this(),
                      std::placeholders::_1, std::placeholders::_2));
    }
    
}