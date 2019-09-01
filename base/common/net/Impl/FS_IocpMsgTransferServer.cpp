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
 * @file  : FS_IocpMsgTransferServer.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/5
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/net/Impl/FS_IocpMsgTransferServer.h"
#include "base/common/net/Impl/FS_Iocp.h"
#include "base/common/net/Impl/FS_Client.h"
#include "base/common/net/Defs/IocpDefs.h"
#include "base/common/net/Defs/FS_NetDefs.h"
#include "base/common/net/Defs/FS_NetBuffer.h"
#include "base/common/net/Impl/FS_Iocp.h"
#include "base/common/net/Interface/INetEvent.h"
#include "base/common/log/Log.h"

FS_NAMESPACE_BEGIN

OBJ_POOL_CREATE_IMPL(FS_IocpMsgTransferServer, _objPoolHelper, __DEF_OBJ_POOL_OBJ_NUM__)

FS_IocpMsgTransferServer::FS_IocpMsgTransferServer()
    :_iocpClientMsgTransfer(new FS_Iocp)
    ,_ioEvent(new IO_EVENT)
{
    _iocpClientMsgTransfer->Create();
}

FS_IocpMsgTransferServer::~FS_IocpMsgTransferServer()
{
    Close();
    Fs_SafeFree(_iocpClientMsgTransfer);
    Fs_SafeFree(_ioEvent);
}

void FS_IocpMsgTransferServer::BeforeClose()
{
    g_Log->net<FS_IocpMsgTransferServer>("FS_IocpMsgTransferServer%d.BeforeClose begin quit iocp", _id);
    // 退出iocp
    _iocpClientMsgTransfer->PostQuit();
}

Int32 FS_IocpMsgTransferServer::_BeforeClientMsgTransfer()
{
    // 1.遍历post 客户端请求
    FS_Client *client = NULL;
    for(auto iter = _clientIdRefClients.begin(); iter != _clientIdRefClients.end();)
    {
        client = iter->second;
        if(client->IsDestroy())
        {
            g_Log->w<FS_IocpMsgTransferServer>(_LOGFMT_("clientId[%llu] is destroyed"), iter->first);
            ++iter;
            continue;
        }

        // 需要写数据的客户端,才postSend
        if(!client->IsPostSend() && client->NeedWrite())
        {
            auto ioData = client->MakeSendIoData();
            if(ioData)
            {
                if(_iocpClientMsgTransfer->PostSend(ioData) != StatusDefs::Success)
                {
                    g_Log->net<FS_IocpMsgTransferServer>("_BeforeClientMsgTransfer postsend fail clientid[%llu] sock[%llu]"
                                                         , client->GetId(), client->GetSocket());

                    iter = _OnClientLeaveAndEraseFromQueue(iter);
                    continue;
                }
            }

            ioData = client->MakeRecvIoData();
            if(ioData)
            {
                if(_iocpClientMsgTransfer->PostRecv(ioData) != StatusDefs::Success)
                {
                    g_Log->net<FS_IocpMsgTransferServer>("_BeforeClientMsgTransfer PostRecv fail clientid[%llu] sock[%llu]"
                                                         , client->GetId(), client->GetSocket());
                    iter = _OnClientLeaveAndEraseFromQueue(iter);
                    continue;
                }
            }
        }
        else if(!client->IsPostRecv())
        {
            // TODO:需要考虑在1ms内客户端是否需要多次recv，只有没投递过recv的客户端才可以投递recv，避免多次投递，当客户端销毁后有可能recv才完成
            auto ioData = client->MakeRecvIoData();
            if(ioData)
            {
                if(_iocpClientMsgTransfer->PostRecv(ioData) != StatusDefs::Success)
                {
                    g_Log->net<FS_IocpMsgTransferServer>("_BeforeClientMsgTransfer PostRecv fail clientid[%llu] sock[%llu]"
                                                         , client->GetId(), client->GetSocket());
                    iter = _OnClientLeaveAndEraseFromQueue(iter);
                    continue;
                }
            }
        }
        ++iter;
    }

    // 2.iocp等待消息完成直到timeout或error为止
    Int32 ret = StatusDefs::Success;
    while(true)
    {// 只处理到wait超时（意味着这超时后若客户端有数据到来只能等到下次投递完recv）
        ret = _ListenIocpNetEvents();
        if(ret == StatusDefs::IOCP_WaitTimeOut)
            return StatusDefs::Success;
        else if(ret != StatusDefs::Success)
            return ret;
    }

    return StatusDefs::Success;
}

Int32 FS_IocpMsgTransferServer::_ListenIocpNetEvents()
{
    auto ret = _iocpClientMsgTransfer->WaitForCompletion(*_ioEvent, 1);
    if(ret != StatusDefs::Success)
    {
        if(ret != StatusDefs::IOCP_WaitTimeOut)
            g_Log->net<FS_IocpMsgTransferServer>("FS_IOCPServer%d.DoIocpNetEvents.wait nothing but ret[%d]", _id, ret);
        // g_Log->any<FS_IocpMsgTransferServer>("FS_IOCPServer%d.DoIocpNetEvents.wait nothing but ret[%d]", _id, ret);
        return ret;
    }

    // 处理iocp退出
    if(_ioEvent->_data._code == IocpDefs::IO_QUIT)
    {
        g_Log->sys<FS_IocpMsgTransferServer>(_LOGFMT_("iocp退出 code=%lld"), _ioEvent->_data._code);
        return StatusDefs::IOCP_Quit;
    }

    // 判断client是否已被移除
    const UInt64 clientId = _ioEvent->_data._clientId;
    auto client = _GetClient(clientId);
    if(!client)
    {
        g_Log->net<FS_IocpMsgTransferServer>("clientId[%llu] is removed before", clientId);
        return ret;
    }

    if(IocpDefs::IO_RECV == _ioEvent->_ioData->_ioType)
    {// 接收数据 完成 Completion

        if(_ioEvent->_bytesTrans <= 0)
        {// 客户端断开处理
            g_Log->any<FS_IocpMsgTransferServer>("client sock[%llu] clientId[%llu] IO_TYPE::RECV bytesTrans[%d]"
                                                 , client->GetSocket()
                                                 , clientId,
                                                 _ioEvent->_bytesTrans);

            //CELLLog_Info("rmClient sockfd=%d, IO_TYPE::RECV bytesTrans=%d", _ioEvent.pIoData->sockfd, _ioEvent.bytesTrans);
            // _RmClient(*_ioEvent);
            _RmClient(client);
            return ret;
        }

        // 判断是否断开 已断开的有可能是上次postsend只后1ms内系统没有完成send，同时客户端被移除导致
        if(client->IsDestroy())
        {
            g_Log->e<FS_IocpMsgTransferServer>(_LOGFMT_("IO_RECV clientId[%llu] is destroy")
                                                , client->GetId());
        }

        client->OnRecvFromIocp(_ioEvent->_ioData->_owner->GetNode(), _ioEvent->_bytesTrans);
        _OnPrepareNetRecv(client);
    }
    else if(IocpDefs::IO_SEND == _ioEvent->_ioData->_ioType)
    {// 发送数据 完成 Completion

        if(_ioEvent->_bytesTrans <= 0)
        {// 客户端断开处理
            g_Log->any<FS_IocpMsgTransferServer>("client sock[%llu] clientId[%llu] IO_TYPE::IO_SEND bytesTrans[%d]"
                                                 , client->GetSocket()
                                                 , clientId
                                                 , _ioEvent->_bytesTrans);

            _RmClient(client);
            return ret;
        }

        // 判断是否断开 已断开的有可能是上次postsend只后1ms内系统没有完成send，同时客户端被移除导致
        if(client->IsDestroy())
        {
            g_Log->e<FS_IocpMsgTransferServer>(_LOGFMT_("IO_SEND clientId[%llu] is destroy")
                                                , client->GetId());
        }

        client->OnSend2iocp(_ioEvent->_ioData->_owner->GetNode(), _ioEvent->_bytesTrans);
    }
    else 
    {
        g_Log->e<FS_IocpMsgTransferServer>(_LOGFMT_("undefine io type[%d]."), _ioEvent->_ioData->_ioType);
    }

    return ret;
}

void FS_IocpMsgTransferServer::_OnClientJoin(FS_Client *client)
{
    // 玩家连入时监听玩家数据
    _iocpClientMsgTransfer->Reg(client->GetSocket(), client->GetId());
    auto ioData = client->MakeRecvIoData();
    if(ioData)
    {
        if(_iocpClientMsgTransfer->PostRecv(ioData) != StatusDefs::Success)
        {
            g_Log->net<FS_IocpMsgTransferServer>("_OnClientJoin PostRecv fail clientid[%llu] sock[%llu]"
                                                 , client->GetId(), client->GetSocket());
            client->Close();
        }
    }
}

FS_NAMESPACE_END
