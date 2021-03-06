/*!
 * MIT License
 *
 * Copyright (c) 2019 ericyonng<120453674@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file  : FS_IocpSessionImpl.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/14
 * @brief :
 * 
 *
 * 
 */

#ifdef __Frame_Include_FrightenStone_Common_Net_Impl_FS_IocpSession_H__
#pragma once

#ifdef _WIN32

FS_NAMESPACE_BEGIN

inline FS_IocpSession::FS_IocpSession(UInt64 sessionId
                                      , UInt32 transferCompId
                                      , UInt32 acceptorCompId
                                      , SOCKET sock
                                      , const sockaddr_in *addrInfo
                                      , IMemoryAlloctor *memAlloctor
                                      , Int64 heartbeatIntervalMicroSeconds)
    :IFS_Session(sessionId, transferCompId, acceptorCompId,  sock, addrInfo, memAlloctor, heartbeatIntervalMicroSeconds)
    ,_isPostRecv(false)
    ,_isPostSend(false)
    ,_iocp(NULL)
{
}

inline void FS_IocpSession::ResetPostRecvMask()
{
    _isPostRecv = false;
}

inline void FS_IocpSession::ResetPostSendMask()
{
    _isPostSend = false;
}

inline void FS_IocpSession::EnableDisconnect()
{
    _isPostSend = false;
    _isPostRecv = false;
}

inline void FS_IocpSession::Bind(FS_Iocp *iocp)
{
    _iocp = iocp;
}

inline FS_IocpBuffer *FS_IocpSession::CastToRecvBuffer()
{
    return reinterpret_cast<FS_IocpBuffer *>(_recvBuffer);
}

inline bool FS_IocpSession::IsPostIoChange() const
{
    return _isPostRecv || _isPostSend;
}

inline bool FS_IocpSession::IsPostSend() const
{
    return _isPostSend;
}

inline bool FS_IocpSession::IsPostRecv() const
{
    return _isPostRecv;
}

inline bool FS_IocpSession::CanPost() const
{
    return !(_isDestroy || _maskClose || IsClose());
}

inline bool FS_IocpSession::CanDestroy() const
{
    return !IsPostIoChange();
}

inline bool FS_IocpSession::CanDisconnect() const
{
    return !IsPostIoChange();
}

inline void FS_IocpSession::OnSendSuc(Int64 transferBytes, IoDataBase *ioData)
{// 注意 _node 属于ioData，而ioData是buffer的数据成员，故若先释放buffer就会直接释放ioData， node就会被直接释放，所以应该先移除list中的节点再释放buffer
    _isPostSend = false;
    ioData->_callback->Invoke(transferBytes);
    auto bufferNode = ioData->_node;
    auto buffer = *bufferNode->_iterNode;
    if(buffer->IsEmpty())
    {
        _toSend.erase(bufferNode->_iterNode);
        Fs_SafeFree(buffer);
    }
}

inline void FS_IocpSession::OnRecvSuc(Int64 transferBytes, IoDataBase *ioData)
{
    _isPostRecv = false;
    ioData->_callback->Invoke(transferBytes);
}

FS_NAMESPACE_END

#endif

#endif
