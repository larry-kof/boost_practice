#ifndef RPC_CODEC_H
#define RPC_CODEC_H

#include <memory>
#include <functional>
#include <list>
#include <mutex>
#include <thread>
#include "common/net.hpp"
#include "common/Buffer.h"

namespace bean
{
namespace net
{
class RpcMessage;
typedef std::shared_ptr<RpcMessage> RpcMessagePtr;
typedef std::shared_ptr<std::vector<char>> ReadBufferPtr;
class rpc_session;
class rpc_codec
{
public:
    enum CodecError
    {
        kNoError = 0,
        kChecksumError = 1,
    };

    typedef std::function<void(const RpcMessagePtr &rpcMsg, CodecError error)> RpcCallbackFunc;
    rpc_codec(RpcCallbackFunc &&cb);

    void on_read(size_t bytes, const ReadBufferPtr &readBuffer);

    void send_rpc_msg(const std::shared_ptr<rpc_session> &session, const RpcMessagePtr &rpcMsg);

private:
    struct BufferValue {
    public:
        bool valid;
        std::shared_ptr<Buffer> buffer;
        BufferValue():valid(true), buffer(std::make_shared<Buffer>()){}
    };

    bool validate_checksum(const char *buf, int32_t len);
    void on_write(const boost::system::error_code &ec, size_t bytes, const std::shared_ptr<rpc_session> &session, std::list<BufferValue>::pointer bufferValue);
    Buffer inputBuffer_;
    std::list< BufferValue > outputBuffers_;
    std::mutex mutexBuffers_;
    RpcCallbackFunc rpcCallback_;

    const int kMinMessageLen = sizeof(int32_t) + sizeof(int32_t);
};

} // namespace net
} // namespace bean

#endif