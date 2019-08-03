//
//  http_session.cpp
//  boost_practice
//
//  Created by larry-kof on 2019/7/28.
//  Copyright © 2019 larry-kof. All rights reserved.
//

#include "http_session.hpp"


beast::string_view
mime_type(beast::string_view path)
{
    using beast::iequals;
    auto const ext = [&path]
    {
        auto const pos = path.rfind(".");
        if(pos == beast::string_view::npos)
            return beast::string_view{};
        return path.substr(pos);
    }();
    if(iequals(ext, ".htm"))  return "text/html";
    if(iequals(ext, ".html")) return "text/html";
    if(iequals(ext, ".php"))  return "text/html";
    if(iequals(ext, ".css"))  return "text/css";
    if(iequals(ext, ".txt"))  return "text/plain";
    if(iequals(ext, ".js"))   return "application/javascript";
    if(iequals(ext, ".json")) return "application/json";
    if(iequals(ext, ".xml"))  return "application/xml";
    if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if(iequals(ext, ".flv"))  return "video/x-flv";
    if(iequals(ext, ".png"))  return "image/png";
    if(iequals(ext, ".jpe"))  return "image/jpeg";
    if(iequals(ext, ".jpeg")) return "image/jpeg";
    if(iequals(ext, ".jpg"))  return "image/jpeg";
    if(iequals(ext, ".gif"))  return "image/gif";
    if(iequals(ext, ".bmp"))  return "image/bmp";
    if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if(iequals(ext, ".tiff")) return "image/tiff";
    if(iequals(ext, ".tif"))  return "image/tiff";
    if(iequals(ext, ".svg"))  return "image/svg+xml";
    if(iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}

std::string
path_cat(
         beast::string_view base,
         beast::string_view path)
{
    if(base.empty())
        return std::string(path);
    std::string result(base);
    char constexpr path_separator = '/';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    return result;
}

http_session::http_session(tcp::socket&& socket, std::string root_doc)
:socket_(std::move(socket)), root_doc_(root_doc), strand_(socket.get_executor())
{
    
}

void http_session::run()
{
    do_read();
}

struct http_session::send_lamda
{
private:
    http_session& self_;
public:
    send_lamda(http_session& self):self_(self){}
    
    template <bool isRequest, class Body, class Allocator>
    void operator()(http::message<isRequest, Body, Allocator>&& msg)
    {
        auto sp = boost::make_shared<http::message<isRequest, Body, Allocator>>(std::move(msg));
        
        auto self = self_.shared_from_this();
        http::async_write(self->socket_, *sp, [self, sp](beast::error_code ec, size_t bytes){
            self->on_write(ec, bytes, sp->need_eof());
        });
    }
};

template <class Body, class Allocator, class Send>
void handle_request( beast::string_view doc_root,
                    http::request<Body, http::basic_fields<Allocator>>&& req,
                    Send&& send)
{
    const auto bad_request = [&req](std::string why) {
        http::response<http::string_body> res{http::status::bad_request, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "An error occurred: '" + why + "'";
        res.prepare_payload();
        return res;
    };
    
    auto not_found = [&req](beast::string_view path) {
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        http::string_body::value_type body;
        body = path.to_string() + " is not found";
        res.keep_alive(false);
        res.body() = std::move(body);
        res.prepare_payload();
        return res;
    };
    
    
    if( req.method() != http::verb::get  && req.method() != http::verb::head )
    {
        return send(bad_request("Unknown http method"));
    }
    
    if( req.target().empty() ||
       req.target()[0] != '/' ||
       req.target().find("..") != beast::string_view::npos)
        return send(bad_request("Illegal request-target"));
    
    std::string path = path_cat(doc_root, req.target());
    if(req.target().back() == '/')
        path.append("index.html");
    
    beast::error_code ec;
    http::file_body::value_type body;
    body.open(path.c_str(), beast::file_mode::scan, ec);
    if(ec == boost::system::errc::no_such_file_or_directory) {
        return send(not_found(req.target()));
    }
    
    auto const size = body.size();
    
    http::response<http::file_body> res{ http::status::ok, req.version() };
    res.body() = std::move(body);
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    res.prepare_payload();
    return send(std::move(res));
    
}


void http_session::do_read()
{
    parser_.emplace();
    
    parser_->body_limit(1000);
    
    http::async_read(socket_, buffer_, parser_->get(), net::bind_executor(strand_, std::bind(&http_session::on_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2 )));
}

void http_session::on_write(beast::error_code ec, size_t bytes, bool close)
{
    if(ec)
    {
        std::cout<< ec.message() << std::endl;
        return;
    }
    
    if(close)
    {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        socket_.shutdown(tcp::socket::shutdown_send, ec);
        return;
    }
    
    // Read another request
    do_read();
}

void http_session::on_read(beast::error_code ec, size_t bytes)
{
    if(ec == http::error::end_of_stream)
    {
        socket_.shutdown(tcp::socket::shutdown_send);
        return;
    }
    
    if(ec) {
        std::cout<< ec.message() << std::endl;
        return;
    }
    
    handle_request(root_doc_, parser_->release(), send_lamda(*this));
}
