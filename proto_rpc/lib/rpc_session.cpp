
#include "rpc_session.h"
#include "rpc.pb.h"
#include <google/protobuf/descriptor.h>
#include <iostream>

using namespace bean::net;

#ifdef USE_SSL
rpc_session::rpc_session(tcp::socket &&socket, boost_net::ssl::context& context, SessionOverFunc sessionOver, SSLComplete sslComplete)
    : strand_(socket.get_executor()), timeOut_(socket.get_io_context()), socket_(std::move(socket), context),sslComplete_(std::move(sslComplete)),
#else
rpc_session::rpc_session(tcp::socket &&socket, SessionOverFunc sessionOver)
    : strand_(socket.get_executor()), timeOut_(socket.get_io_context()), socket_(std::move(socket)),
#endif
      codec_(std::bind(&rpc_session::on_rpc_callback, this, std::placeholders::_1,
                       std::placeholders::_2)),
      services_(nullptr), id_(0), sessionOverFunc_(std::move(sessionOver))
{
}

#ifdef USE_SSL
void rpc_session::do_handshake(bool isServer)
{
    auto self = shared_from_this();
    auto stream_type = isServer ? boost_net::ssl::stream_base::server : boost_net::ssl::stream_base::client;
    socket_.async_handshake(stream_type,
    [this, self](const boost::system::error_code& ec){
        if(ec)
        {
            fail(ec ,"ssl hand shake");
            if(sslComplete_) sslComplete_(false);
            return;
        }
        if(sslComplete_)
        {
            sslComplete_(true);
        }
        do_read();
    });
}
#endif

void rpc_session::run(bool isServer) 
{ 
#ifdef USE_SSL
    if( !isServer)
    {
        socket_.set_verify_mode(boost::asio::ssl::verify_peer);
        socket_.set_verify_callback([](bool preverified, boost_net::ssl::verify_context& ctx)
        {
            char subject_name[256];
            X509 *cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
            X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
            std::cout << "Verifying " << subject_name << "\n";

            return preverified;
        });
    }
    do_handshake(isServer);
#else
    do_read(); 
#endif
}

void rpc_session::do_read()
{
    auto readBuffer = std::make_shared<std::vector<char>>(100);
    socket_.async_read_some(
        boost_net::buffer(*readBuffer),
        boost_net::bind_executor(strand_, std::bind(&rpc_session::on_read, shared_from_this(),
                                                    std::placeholders::_1, std::placeholders::_2,
                                                    readBuffer)));
    timeOut_.expires_after(std::chrono::seconds(2));
}

void rpc_session::time_out(const boost::system::error_code &ec)
{
    if (!ec)
    {
        // time out
#ifdef USE_SSL
        if (socket_.lowest_layer().is_open())
#else
        if (socket_.is_open())
#endif
        {
            boost::system::error_code socketEc;
#ifdef USE_SSL
            socket_.lowest_layer().shutdown(boost_net::socket_base::shutdown_both, socketEc);
#else
            socket_.shutdown(boost_net::socket_base::shutdown_both, socketEc);
#endif
            if (socketEc && socketEc.value() != boost::asio::error::not_connected)
            {
                fail(socketEc, "shutdown");
            }
        }
        std::cerr << " time out " << std::endl;
        sessionOverFunc_(shared_from_this());
    }
    else if (ec.value() == boost::asio::error::operation_aborted)
    {
    }
}

void rpc_session::on_read(const boost::system::error_code &ec, size_t bytes,
                          const ReadBufferPtr &readBuffer)
{
    timeOut_.cancel();
    if (ec)
    {
#ifdef USE_SSL
        if (socket_.lowest_layer().is_open())
#else
        if (socket_.is_open())
#endif
        {
            if (ec.value() == boost::asio::error::eof)
            {
                try
                {
#ifdef USE_SSL
                    std::cout << " disconect from " << socket_.lowest_layer().remote_endpoint() << std::endl;
#else
                    std::cout << " disconect from " << socket_.remote_endpoint() << std::endl;
#endif

                }
                catch (const std::exception &e)
                {
                    std::cout << e.what() << std::endl;
                }
            }
            boost::system::error_code socketEc;
#ifdef USE_SSL
            socket_.lowest_layer().shutdown(boost_net::socket_base::shutdown_send, socketEc);
#else
            socket_.shutdown(boost_net::socket_base::shutdown_send, socketEc);
#endif
            if (socketEc && socketEc.value() != boost::asio::error::not_connected)
            {
                fail(socketEc, "shutdown");
            }
        }
        sessionOverFunc_(shared_from_this());
        return;
    }
    codec_.on_read(bytes, readBuffer);
    do_read();
}

void rpc_session::disconnect()
{
#ifdef USE_SSL
    if (socket_.lowest_layer().is_open())
#else
    if (socket_.is_open())
#endif
    {
        boost::system::error_code ec;
#ifdef USE_SSL
        socket_.lowest_layer().shutdown(boost_net::socket_base::shutdown_send, ec);
#else
        socket_.shutdown(boost_net::socket_base::shutdown_send, ec);
#endif
        if (ec)
        {
            fail(ec, "shutdown");
        }
    }
}

void rpc_session::on_rpc_callback(const RpcMessagePtr &rpcMsg, rpc_codec::CodecError error)
{
    ErrorCode errorCode = NO_ERROR;
    if (error != rpc_codec::kNoError)
    {
        errorCode = INVALID_CHECKSUM;
    }
    else if (rpcMsg->type() == MessageType::REQUEST)
    {
        std::string serviceName = rpcMsg->service();
        std::string methodName = rpcMsg->method();
        if (services_)
        {
            auto itService = services_->find(serviceName);
            if (itService != services_->end())
            {
                google::protobuf::Service *service = itService->second;
                const google::protobuf::MethodDescriptor *method =
                    service->GetDescriptor()->FindMethodByName(methodName);

                if (method)
                {
                    std::unique_ptr<google::protobuf::Message> request(
                        service->GetRequestPrototype(method).New());

                    if (request->ParseFromString(rpcMsg->request()))
                    {
                        google::protobuf::Message *response =
                            service->GetResponsePrototype(method).New();
                        int32_t id = rpcMsg->id();
                        service->CallMethod(
                            method, NULL, request.get(), response,
                            NewCallback(this, &rpc_session::done_callback, response, id));
                        errorCode = NO_ERROR;
                    }
                    else
                    {
                        errorCode = INVALID_REQUEST;
                    }
                }
                else
                {
                    errorCode = NO_METHOD;
                }
            }
            else
            {
                errorCode = NO_SERVICE;
            }
        }
        else
        {
            errorCode = NO_SERVICE;
        }
    }
    else if (rpcMsg->type() == MessageType::RESPONSE)
    {
        int32_t id = rpcMsg->id();
        google::protobuf::Message *response = NULL;
        google::protobuf::Closure *done = NULL;
        {
            std::lock_guard<std::mutex> lck(mutex_);
            auto it = reqCallbacks_.find(id);
            if (it != reqCallbacks_.end())
            {
                std::tie(response, done) = it->second;
                reqCallbacks_.erase(it);
            }
        }

        if (response)
        {
            int resError = rpcMsg->error();
            if (resError != NO_ERROR)
            {
                std::cerr << "error " << resError << std::endl;
            }
            else
            {
                response->ParseFromString(rpcMsg->response());
                if (done)
                {
                    done->Run();
                }
            }
        }
    }

    if (errorCode != NO_ERROR)
    {
        std::shared_ptr<RpcMessage> rpcResMsg(RpcMessage::default_instance().New());
        rpcResMsg->set_id(rpcMsg->id());
        rpcResMsg->set_error(errorCode);
        rpcResMsg->set_type(RESPONSE);

        codec_.send_rpc_msg(shared_from_this(), rpcResMsg);
    }
}

void rpc_session::done_callback(google::protobuf::Message *response, int32_t id)
{
    std::unique_ptr<google::protobuf::Message> d(response);
    std::shared_ptr<RpcMessage> rpcMsg(RpcMessage::default_instance().New());
    rpcMsg->set_id(id);
    std::string resStr = response->SerializeAsString();
    if (resStr.empty())
    {
        rpcMsg->set_type(RESPONSE);
        rpcMsg->set_error(INVALID_REQUEST);
    }
    else
    {
        rpcMsg->set_type(RESPONSE);
        rpcMsg->set_error(NO_ERROR);
        rpcMsg->set_response(std::move(resStr));
    }
    codec_.send_rpc_msg(shared_from_this(), rpcMsg);
}

void rpc_session::set_services(const std::map<std::string, google::protobuf::Service *> *services)
{
    services_ = services;
}

void rpc_session::CallMethod(const google::protobuf::MethodDescriptor *method,
                             google::protobuf::RpcController *controller,
                             const google::protobuf::Message *request,
                             google::protobuf::Message *response, google::protobuf::Closure *done)
{
    std::shared_ptr<RpcMessage> rpcMsg(RpcMessage::default_instance().New());
    int32_t id = id_.fetch_add(1);
    rpcMsg->set_id(id);
    rpcMsg->set_type(REQUEST);
    rpcMsg->set_service(method->service()->full_name());
    rpcMsg->set_method(method->name());
    rpcMsg->set_request(request->SerializeAsString());

    {
        std::lock_guard<std::mutex> lck(mutex_);
        reqCallbacks_[id] = std::make_tuple(response, done);
    }

    codec_.send_rpc_msg(shared_from_this(), rpcMsg);
}