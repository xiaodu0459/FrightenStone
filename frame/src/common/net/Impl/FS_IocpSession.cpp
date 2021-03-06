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
 * @file  : FS_IocpSession.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/14
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "FrightenStone/common/net/Impl/FS_IocpSession.h"
#include "FrightenStone/common/net/Defs/FS_IocpBuffer.h"
#include "FrightenStone/common/net/Impl/FS_Iocp.h"
#include "FrightenStone/common/net/Impl/FS_Addr.h"

#include "FrightenStone/common/log/Log.h"

#ifdef _WIN32

FS_NAMESPACE_BEGIN
OBJ_POOL_CREATE_DEF_IMPL(FS_IocpSession, __DEF_OBJ_POOL_OBJ_NUM__);

FS_IocpSession::~FS_IocpSession()
{
    if(!CanDestroy())
        g_Log->e<FS_IocpSession>(_LOGFMT_("has io uncompleted please check"));

}

// 
// IoDataBase *FS_IocpSession::MakeSendIoData()
// {
//     if(!CanPost())
//         return NULL;
// 
//     if(_isPostSend || _toSend.empty())
//         return NULL;
// 
//     _isPostSend = true;
//     auto buffer = _toSend.front()->CastToBuffer<FS_IocpBuffer>();
//     auto ioData = buffer->MakeSendIoData();
//     if(!ioData)
//         return NULL;
//     
//     if(!ioData->_node)
//         ioData->_node = new BufferQueueNode;
// 
//     auto node = ioData->_node;
//     node->_isPost = true;
//     node->_iterNode = _toSend.begin();
//     return ioData;
// }

Int32 FS_IocpSession::PostRecv()
{
    _isPostRecv = true;
    auto ioData = _recvBuffer->CastToBuffer<FS_IocpBuffer>()->MakeRecvIoData();
    if(!ioData)
    {
        g_Log->w<FS_IocpSession>(_LOGFMT_("make recv io data fail sessionId[%llu] sock[%llu]"), _sessionId, _sock);
        return StatusDefs::Success;
    }

    Int32 st = _iocp->PostRecv(_sock, ioData);
    if(st != StatusDefs::Success)
    {
        ResetPostRecvMask();
        if(st != StatusDefs::IOCP_ClientForciblyClosed)
        {
            g_Log->e<FS_IocpSession>(_LOGFMT_("sessionId[%llu] socket[%llu] post recv fail st[%d]")
                                     , _sessionId, _sock, st);
        }

        // 这个时候也有可能post刚好完成，所以不宜用closesocket，而是cacelio，使得已完成的不会被取消，且到此session即将关闭
        MaskClose();
        CancelIoEx(HANDLE(_sock), NULL);
        return st;
    }

    return StatusDefs::Success;
}

Int32 FS_IocpSession::PostSend()
{
    _isPostSend = true;
    auto buffer = _toSend.front()->CastToBuffer<FS_IocpBuffer>();
    auto ioData = buffer->MakeSendIoData();
    if(!ioData)
    {
        ResetPostSendMask();
        g_Log->w<FS_IocpSession>(_LOGFMT_("make send io data fail sessionId[%llu] sock[%llu]"), _sessionId, _sock);
        return StatusDefs::Success;
    }

    if(!ioData->_node)
        ioData->_node = new BufferQueueNode;

    auto node = ioData->_node;
    node->_iterNode = _toSend.begin();
    Int32 st = _iocp->PostSend(_sock, ioData);
    if(st != StatusDefs::Success)
    {
        ResetPostSendMask();
        if(st != StatusDefs::IOCP_ClientForciblyClosed)
        {
            g_Log->e<FS_IocpSession>(_LOGFMT_("sessionId[%llu] socket[%llu] post send fail st[%d]")
                                         , _sessionId, _sock, st);
        }

        // 这个时候也有可能post刚好完成，所以不宜用closesocket，而是cacelio，使得已完成的不会被取消，且到此session即将关闭
        MaskClose();
        CancelIoEx(HANDLE(_sock), NULL);
        return st;
    }
    
    return StatusDefs::Success;
}

void FS_IocpSession::CancelPostedEventsAndMaskClose()
{
    // 不用closesocket，而是cacelio，使得已完成的不会被取消，且到此session即将关闭
    MaskClose();
    CancelIoEx(HANDLE(_sock), NULL);
}

// 客户端连入
void FS_IocpSession::OnConnect()
{
    IFS_Session::OnConnect();

    // 绑定iocp
    Int32 st = _iocp->Reg(_sock, _sessionId);
    if(st != StatusDefs::Success)
    {
        g_Log->e<FS_IocpSession>(_LOGFMT_("iocp reg sock[%llu] sessionId[%llu] addr<%s> fail st[%d]")
                                 , _sock, _sessionId, _addr->ToString().c_str(), st);
    }
}

FS_NAMESPACE_END

#endif
