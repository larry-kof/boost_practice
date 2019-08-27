#include "rpc_codec.h"
#include "rpc.pb.h"
#include <zlib.h>
#include <tuple>
#include "rpc_session.h"

using namespace bean::net;

rpc_codec::rpc_codec(RpcCallbackFunc &&cb)
    : rpcCallback_(std::move(cb))
{
    outputBuffers_.emplace_back();
}

void rpc_codec::on_read(size_t bytes, const ReadBufferPtr &readBuffer)
{
    inputBuffer_.append(&*(readBuffer->begin()), bytes);
    while (inputBuffer_.readableBytes() >= kMinMessageLen)
    {
        int32_t len = inputBuffer_.peekInt32();
        if (inputBuffer_.readableBytes() >= sizeof(int32_t) + len)
        {
            CodecError error = kNoError;
            inputBuffer_.retrieve(sizeof(len));
            bool validate = validate_checksum(inputBuffer_.peek(), len);
            if (!validate)
            {
                error = kChecksumError;
                rpcCallback_(nullptr, error);
            }
            else
            {
                RpcMessagePtr rpcMsgPtr(RpcMessage::default_instance().New());
                rpcMsgPtr->ParseFromArray(inputBuffer_.peek(), len - sizeof(int32_t));
                rpcCallback_(rpcMsgPtr, error);
            }
            inputBuffer_.retrieve(len);
        }
        else
        {
            break;
        }
    }
}

bool rpc_codec::validate_checksum(const char *buf, int32_t len)
{
    int32_t checksum = adler32(1, static_cast<const Bytef *>((const void *)buf), len - sizeof(int32_t));
    int32_t expectedChecksum = *(int32_t *)(buf + len - sizeof(int32_t));
    expectedChecksum = NTOHL(expectedChecksum);

    return checksum == expectedChecksum;
}

void rpc_codec::send_rpc_msg(const std::shared_ptr<rpc_session> &session, const RpcMessagePtr &rpcMsg)
{
    int32_t len = rpcMsg->ByteSize();

    Buffer* outputBuffer = NULL;
    std::list<BufferValue>::pointer bufferValue = NULL;
    {
        std::lock_guard<std::mutex> lck(mutexBuffers_);
        for(auto it = outputBuffers_.begin(); it != outputBuffers_.end(); it++)
        {
            if(it->valid)
            {
                it->valid = false;
                bufferValue = &*it;
                outputBuffer = bufferValue->buffer.get();
                break;
            }
        }
        if(outputBuffer == NULL)
        {
            outputBuffers_.emplace_back();
            std::list<BufferValue>::reference newBufferValueRef = outputBuffers_.back();
            newBufferValueRef.valid = false;
            bufferValue = &newBufferValueRef;
            outputBuffer = bufferValue->buffer.get();
        }
    }

    outputBuffer->ensureWritableBytes(len);
    uint8_t *start = reinterpret_cast<uint8_t *>(outputBuffer->beginWrite());
    rpcMsg->SerializeWithCachedSizesToArray(start);
    outputBuffer->hasWritten(len);

    int32_t checksum = adler32(1, static_cast<const Bytef *>((const void *)outputBuffer->peek()), outputBuffer->readableBytes());
    outputBuffer->appendInt32(checksum);
    outputBuffer->prependInt32(len + sizeof(checksum));

    session->socket_.async_write_some(boost_net::buffer(outputBuffer->peek(), outputBuffer->readableBytes()),
                                      std::bind(&rpc_codec::on_write, this, std::placeholders::_1, std::placeholders::_2, session, bufferValue));
}

void rpc_codec::on_write(const boost::system::error_code &ec, size_t bytes, const std::shared_ptr<rpc_session> &session, std::list<BufferValue>::pointer bufferValue)
{
    if (ec)
    {
        fail(ec, "on write");
        session->socket_.shutdown(boost_net::socket_base::shutdown_send);
        return;
    }
    bufferValue->buffer->retrieve(bytes);
    if (bufferValue->buffer->readableBytes() > 0)
    {
        session->socket_.async_write_some(boost_net::buffer(bufferValue->buffer->peek(), bufferValue->buffer->readableBytes()),
                                          std::bind(&rpc_codec::on_write, this, std::placeholders::_1, std::placeholders::_2, session, bufferValue));
    }
    else if (bufferValue->buffer->readableBytes() == 0)
    {
        bufferValue->valid = true;
    }
}