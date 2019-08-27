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
 * @file  : FS_Server.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/3
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/net/Impl/FS_Server.h"
#include "base/common/log/Log.h"
#include "base/common/socket/socket.h"
#include "base/common/component/Impl/TimeSlice.h"
#include "base/common/net/Impl/FS_Client.h"
#include "base/common/assist/utils/Impl/STLUtil.h"

FS_NAMESPACE_BEGIN

OBJ_POOL_CREATE_IMPL(FS_Server, _objPoolHelper, __DEF_OBJ_POOL_OBJ_NUM__)

FS_Server::FS_Server()
    :_threadPool(new FS_ThreadPool(0, 1))
{
}

FS_Server::~FS_Server()
{
    g_Log->net<FS_Server>("FS_Server%d.~FS_Server exit begin", _id);
    g_Log->sys<FS_Server>(_LOGFMT_( "FS_Server%d.~FS_Server exit begin"), _id);
    Close();
    _locker.Lock();
    STLUtil::DelMapContainer(_clientIdRefClients);
    STLUtil::DelVectorContainer(_clientsCache);
    _locker.Unlock();
    g_Log->net<FS_Server>("FS_Server%d.~FS_Server exit begin", _id);
    g_Log->sys<FS_Server>(_LOGFMT_("FS_Server%d.~FS_Server exit begin"), _id);
    Fs_SafeFree(_threadPool);
}

#pragma region misc
void FS_Server::_ClearClients()
{
    _locker.Lock();
    STLUtil::DelMapContainer(_clientIdRefClients);
    STLUtil::DelVectorContainer(_clientsCache);
    _locker.Unlock();
}
#pragma endregion

#pragma region recv/addclient/start/close
Int32 FS_Server::RecvData(FS_Client *client)
{
    // 接收客户端数据
    int len = client->RecvData();
    // 触发<接收到网络数据>事件
    _eventHandleObj->OnPrepareNetRecv(client);
    return len;
}

void FS_Server::Start()
{
    auto clientMsgTrasfer = DelegatePlusFactory::Create(this, &FS_Server::_ClientMsgTransfer);
    _threadPool->AddTask(clientMsgTrasfer);
}

void FS_Server::BeforeClose()
{
}

// 关闭Socket
void FS_Server::Close()
{
    BeforeClose();
    g_Log->net<FS_Server>("FS_Server%d.Close begin", _id);
    g_Log->sys<FS_Server>(_LOGFMT_("FS_Server%d.Close begin"), _id);
    _threadPool->Clear();
    g_Log->net<FS_Server>("FS_Server%d.Close end", _id);
    g_Log->sys<FS_Server>(_LOGFMT_("FS_Server%d.Close end"), _id);
}
#pragma endregion

#pragma region net message handle
void FS_Server::_ClientMsgTransfer(const FS_ThreadPool *pool)
{
    while(!pool->IsClearingPool())
    {
        if(!_clientsCache.empty())
        {// 从缓冲队列里取出客户数据
            _locker.Lock();
            for(auto client : _clientsCache)
            {
                _clientIdRefClients[client->GetId()] = client;
                client->_serverId = _id;
                ++_clientJoin;
                ++_joinClientCnt;
                client->UpdateHeartBeatExpiredTime();
                _AddToHeartBeatQueue(client);
                _eventHandleObj->OnNetJoin(client);
                _OnClientJoin(client);
            }
            _clientsCache.clear();
            _clientsChange = true;
            _locker.Unlock();
        }

        // 如果没有需要处理的客户端，就跳过
        if(_clientIdRefClients.empty())
        {
            // 使用wait
            SocketUtil::Sleep(1);
            continue;
        }

        // TODO:心跳检测优化
        _DetectClientHeartTime();

        // before内部若有大量消息，则可能导致其他客户端心跳超时TODO:
        // TODO关于超级流量导致_BeforeClientMsgTransfer执行过长判定为攻击行为，由外部运维处理攻击
        auto st = _BeforeClientMsgTransfer(_delayRemoveClientIds);
        if(st != StatusDefs::Success)
        {
            g_Log->e<FS_Server>(_LOGFMT_("FS_Server id[%d] _BeforeClientMsgTransfer: st[%d] "), _id, st);

            // 断开的客户端清理(提前清理，时有可能客户端还有数据在getqueue队列中，需要等待iocp消息队列中所有数据都取出才可以清理离线客户端，以免导致崩溃)
            for(auto &client : _delayRemoveClientIds)
                _OnClientLeave(client);

            _delayRemoveClientIds.clear();
            break;
        }

        // 客户端消息到达
        _OnClientMsgArrived();    // TODO关于超级流量导致msgarrive执行过长判定为攻击行为，由外部运维处理攻击

        // 断开的客户端清理(提前清理，时有可能客户端还有数据在getqueue队列中，
        // 需要等待iocp消息队列中所有数据都取出才可以清理离线客户端，以免导致崩溃)
        for(auto &client : _delayRemoveClientIds)
            _OnClientLeave(client);
        _delayRemoveClientIds.clear();
    }

    // 打印当前join的数目以及leave的数目
    g_Log->any<FS_Server>("joined cnt[%lld] leaved cnt[%lld]"
                          , _joinClientCnt, _leaveClientCnt);

    // 退出则清理客户端
    _ClearClients();

    g_Log->net<FS_Server>("FS_Server%d.OnRun exit", _id);
    g_Log->sys<FS_Server>(_LOGFMT_("FS_Server%d.OnRun exit"), _id);
}

void FS_Server::_DetectClientHeartTime()
{
    // 更新当前心跳检测时间戳
    _lastHeartDetectTime.FlushTime();

    // 遍历心跳过期的客户端
    FS_Client *client = NULL;
    for(auto iterClient=_clientHeartBeatQueue.begin();iterClient!=_clientHeartBeatQueue.end();)
    {
        client = *iterClient;
        if(client->GetHeartBeatExpiredTime() > _lastHeartDetectTime)
            break;

#ifdef FS_USE_IOCP
        auto sock = client->GetSocket();
        _delayRemoveClientIds.insert(client->GetId());

        // 若有post消息，则先关闭socket，其他情况不用
        if(client->IsPostIoChange())
            client->Close();
        g_Log->net<FS_Server>("heart beat expired sock[%llu] clientId[%llu] expiredtime[%lld] nowtime[%lld]"
                              , sock, client->GetId(), client->GetHeartBeatExpiredTime().GetMicroTimestamp()
                              ,_lastHeartDetectTime.GetMicroTimestamp());
        // _OnClientLeave(client);
#else
        // _OnClientLeave(client);
        _delayRemoveClients.insert(client);
#endif // CELL_USE_IOCP
        iterClient = _clientHeartBeatQueue.erase(iterClient);
    }
//     UInt16 cpuGroup = 0;
//     Byte8 cpuNum = 0;
//     ULong threadId = SystemUtil::GetCurrentThreadId();
//     SystemUtil::GetCallingThreadCpuInfo(cpuGroup, cpuNum);
//     g_Log->net<FS_Server>("cpuGroup[%hu],cpuNumber[%d],threadId[%lu],fs_server id[%d] _DetectClientHeartTime heart beat queue cnt[%llu]"
//                           , cpuGroup, cpuNum, threadId, _id, _clientHeartBeatQueue.size());
}

void FS_Server::_OnClientLeave(FS_Client *client)
{
    _eventHandleObj->OnNetLeave(client);
    _clientsChange = true;
    _clientHeartBeatQueue.erase(client);
    g_Log->net<FS_Server>("_OnClientLeave sock[%llu] clientId[%llu]", client->GetSocket(), client->GetId());
    delete client;
}

void FS_Server::_OnClientLeave(UInt64 clientId)
{
    auto iterClient = _clientIdRefClients.find(clientId);
    _OnClientLeave(iterClient->second);
    _clientIdRefClients.erase(iterClient);
    ++_leaveClientCnt;
}

void FS_Server::_OnClientJoin(FS_Client *client)
{
    // ...
}

void FS_Server::_OnPrepareNetRecv(FS_Client *client)
{
     _eventHandleObj->OnPrepareNetRecv(client);
}

void FS_Server::_OnClientMsgArrived()
{
    FS_Client *client = NULL;
    for(auto itr : _clientIdRefClients)
    {
        client = itr.second;
//         if(client->IsDestroy())// 要不要销毁不在客户端消息到达的处理范畴
//             continue;

        // 循环 判断是否有消息需要处理
        while(client->HasRecvMsg())
        {
            // 处理网络消息
            auto iterNode = client->FrontMsgNode();
            _HandleNetMsg(client, client->FrontMsg(iterNode));
            // 移除消息队列（缓冲区）最前的一条数据
            client->PopFrontMsg(iterNode);
        }
    }
}

Int32 FS_Server::_HandleNetMsg(FS_Client *client, NetMsg_DataHeader *header)
{
    auto st = _eventHandleObj->OnNetMsg(this, client, header);
    if(st == StatusDefs::Success)
    {
        if(!client->IsDestroy())
        {
            _clientHeartBeatQueue.erase(client);
            client->UpdateHeartBeatExpiredTime();
            _clientHeartBeatQueue.insert(client);
        }
    }
    else
    {
        g_Log->w<FS_Server>(_LOGFMT_("client sock[%llu] heart beat expired time[%lld] error msg st[%d]")
                            , client->GetSocket(), client->GetHeartBeatExpiredTime().GetMicroTimestamp(), st);
    }

    return st;
}
#pragma endregion

FS_NAMESPACE_END


