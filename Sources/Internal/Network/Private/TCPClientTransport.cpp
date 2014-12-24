/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.
 
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
 
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.
 
    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include <Base/FunctionTraits.h>
#include <Debug/DVAssert.h>

#include <Network/Private/TCPClientTransport.h>

namespace DAVA
{
namespace Net
{

TCPClientTransport::TCPClientTransport(IOLoop* aLoop)
    : endpoint()
    , socket(aLoop)
    , listener(NULL)
    , isInitiator(false)
    , isTerminating(false)
    , isConnected(false)
    , sendBufferCount(0)
{
    DVASSERT(aLoop != NULL);
}

TCPClientTransport::TCPClientTransport(IOLoop* aLoop, const Endpoint& aEndpoint)
    : endpoint(aEndpoint)
    , socket(aLoop)
    , listener(NULL)
    , isInitiator(true)
    , isTerminating(false)
    , isConnected(false)
    , sendBufferCount(0)
{
    DVASSERT(aLoop != NULL);
}

TCPClientTransport::~TCPClientTransport()
{
    DVASSERT(NULL == listener && false == isTerminating && false == isConnected);
}

int32 TCPClientTransport::Start(IClientListener* aListener)
{
    DVASSERT(false == isTerminating && aListener != NULL && NULL == listener);
    listener = aListener;
    return DoStart();
}

void TCPClientTransport::Stop()
{
    DVASSERT(listener != NULL && false == isTerminating);
    isTerminating = true;
    CleanUp(0);
}

int32 TCPClientTransport::Send(const Buffer* buffers, size_t bufferCount)
{
    if (false == isConnected) return 0;

    DVASSERT(buffers != NULL && 0 < bufferCount && bufferCount <= SENDBUF_COUNT);
    DVASSERT(true == isConnected && 0 == sendBufferCount);
    for (size_t i = 0;i < bufferCount;++i)
    {
        DVASSERT(buffers[i].base != NULL && buffers[i].len > 0);
        sendBuffers[i] = buffers[i];
    }
    sendBufferCount = bufferCount;

    int32 error = socket.Write(sendBuffers, sendBufferCount, MakeFunction(this, &TCPClientTransport::SocketHandleWrite));
    DVASSERT(0 == error);
    return error;
}

int32 TCPClientTransport::DoStart()
{
    // Try to establish connection if connection is initiated by this
    // Otherwise connection should be already accepted
    int32 error = true == isInitiator ? socket.Connect(endpoint, MakeFunction(this, &TCPClientTransport::SocketHandleConnect))
                                      : DoConnected();
    DVASSERT(0 == error);
    return error;
}

int32 TCPClientTransport::DoConnected()
{
    isConnected = true;
    int32 error = socket.RemoteEndpoint(remoteEndpoint);
    if (0 == error)
        error = socket.StartRead(CreateBuffer(inbuf, sizeof(inbuf)), MakeFunction(this, &TCPClientTransport::SocketHandleRead));
    DVASSERT(0 == error);
    if (0 == error)
        listener->OnTransportConnected(this, remoteEndpoint);
    return error;
}

void TCPClientTransport::CleanUp(int32 error)
{
    if (true == isConnected)
    {
        //DVASSERT(0 == sendBufferCount);
        if (sendBufferCount != 0)
        {
            // Do not forget about buffer that client has "sent"
            listener->OnTransportSendComplete(this);
            sendBufferCount = 0;
        }
        isConnected = false;
        listener->OnTransportDisconnected(this, error);
    }
    socket.Close(MakeFunction(this, &TCPClientTransport::SocketHandleClose));
}

void TCPClientTransport::SocketHandleClose(TCPSocket* socket)
{
    if (true == isTerminating || false == isInitiator)
    {
        IClientListener* p = listener;
        listener = NULL;
        isTerminating = false;
        p->OnTransportTerminated(this); // This can be the last executed line of object instance
    }
    else if (true == isInitiator)
    {
        DoStart();
    }
}

void TCPClientTransport::SocketHandleConnect(TCPSocket* socket, int32 error)
{
    if (true == isTerminating) return;

    0 == error ? DoConnected()
               : DoStart();     // Try one more time
}

void TCPClientTransport::SocketHandleRead(TCPSocket* socket, int32 error, size_t nread)
{
    DVASSERT(false == isTerminating && true == isConnected);
    if (0 == error)
    {
        if (nread != 0)
        {
            listener->OnTransportDataReceived(this, inbuf, nread);
        }
    }
    else
    {
        CleanUp(error);
    }
}

void TCPClientTransport::SocketHandleWrite(TCPSocket* socket, int32 error, const Buffer* buffers, size_t bufferCount)
{
    DVASSERT(false == isTerminating && true == isConnected);
    if (0 == error)
    {
        sendBufferCount = 0;
        listener->OnTransportSendComplete(this);
    }
    else
    {
        CleanUp(error);
    }
}

}   // namespace Net
}   // namespace DAVA
