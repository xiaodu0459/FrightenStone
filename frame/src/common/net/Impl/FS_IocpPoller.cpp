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
 * @file  : FS_IocpPoller.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/12/29
 * @brief :
 */
#include "stdafx.h"
#include <FrightenStone/common/net/Impl/FS_IocpPoller.h>
#include <FrightenStone/common/net/Impl/FS_Iocp.h>
#include "FrightenStone/common/net/Defs/IocpDefs.h"
#include "FrightenStone/common/net/Defs/MessageBlockUtil.h"
#include "FrightenStone/common/net/Defs/NetMethodUtil.h"
#include "FrightenStone/common/net/Defs/IoEvDefs.h"
#include "FrightenStone/common/net/Impl/IFS_EngineComp.h"
#include "FrightenStone/common/net/Impl/IFS_NetEngine.h"
#include "FrightenStone/common/net/Defs/BriefSessionInfo.h"
#include "FrightenStone/common/net/Impl/IFS_MsgTransfer.h"

#include <FrightenStone/common/component/Impl/FS_ThreadPool.h>
#include "FrightenStone/common/log/Log.h"
#include "FrightenStone/common/component/Impl/MessageQueue/MessageQueue.h"
#include "FrightenStone/common/component/Impl/FS_Delegate.h"
#include "FrightenStone/common/socket/socket.h"
#include "FrightenStone/common/net/Defs/EngineCompDefs.h"

#ifdef _WIN32


FS_NAMESPACE_BEGIN
OBJ_POOL_CREATE_DEF_IMPL(FS_IocpPoller, 32);

// poller监视器方法
FS_IocpPoller::MonitorHandler FS_IocpPoller::_monitorHandler[PollerDefs::MonitorType_End] = {
    NULL
    , &FS_IocpPoller::_AcceptorMonitor
    , &FS_IocpPoller::_TransferMonitor
};
Int32 FS_IocpPoller::BeforeStart()
{
    if(_isInit)
        return StatusDefs::Success;

    _pool = new FS_ThreadPool(0, 1);
    _iocp = new FS_Iocp;
    _ioEv = new IoEvent;

    const Int32 st = _iocp->Create();
    if(st != StatusDefs::Success)
    {
        g_Log->e<FS_IocpPoller>(_LOGFMT_("create iocp fail st[%d]"), st);
        return st;
    }

    return StatusDefs::Success;
}

Int32 FS_IocpPoller::Start()
{
    if(_isInit)
        return StatusDefs::Success;

    return StatusDefs::Success;
}

void FS_IocpPoller::AfterStart()
{
    if(_isInit)
        return;

    auto task = DelegatePlusFactory::Create(this, _monitorHandler[_monitorType]);
    if(!_pool->AddTask(task, true))
    {
        g_Log->e<FS_IocpPoller>(_LOGFMT_("addtask fail"));
    }

    _isInit = true;
}

void FS_IocpPoller::WillClose()
{
    if(!_isInit)
        return;

    if(_quitIocpMonitor)
        _quitIocpMonitor->Invoke();

    _pool->Close();
}

void FS_IocpPoller::BeforeClose()
{
    if(!_isInit)
        return;
}

void FS_IocpPoller::Close()
{
    if(!_isInit)
        return;

    _isInit = false;
    FS_Release(_pool);
    Fs_SafeFree(_iocp);
    Fs_SafeFree(_ioEv);
}

IFS_NetModule *FS_IocpPoller::GetNetModuleObj()
{
    return _iocp;
}

void FS_IocpPoller::_TransferMonitor(FS_ThreadPool *pool)
{
    Int32 st = StatusDefs::Success;
    const UInt32 generatorId = _engineComp->CastTo<IFS_MsgTransfer>()->GetGeneratorId();
    const UInt32 compId = _engineComp->GetCompId();

    // 等待其他组件ready
    auto engine = _engineComp->GetEngine();
    _engineComp->MaskReady(true);
    g_Log->sys<FS_IocpPoller>(_LOGFMT_("iocp transfer monitor working..."));

    EngineCompsMethods::WaitForAllCompsReady(engine);
    while(pool->IsPoolWorking())
    {
        st = _iocp->WaitForCompletion(*_ioEv, __FS_POLLER_MONITOR_TIME_OUT_INTERVAL__);

        if(st == StatusDefs::IOCP_WaitTimeOut)
            continue;

        _mq->Push(generatorId, MessageBlockUtil::BuildTransferMonitorArrivedMessageBlock(compId, generatorId, _ioEv, st));
    }

    _engineComp->MaskReady(false);
    _iocp->Destroy();
    g_Log->sys<FS_IocpPoller>(_LOGFMT_("Transfer FS_IocpPoller compId[%u] generatorId[%u] threadId[%llu] end")
                              , compId, generatorId, SystemUtil::GetCurrentThreadId());
}

void FS_IocpPoller::_AcceptorMonitor(FS_ThreadPool *pool)
{
    // 1.监听端口
    _iocp->Reg(_acceptorSockFd);
    _iocp->LoadAcceptEx(_acceptorSockFd);
    auto netEngine = _engineComp->GetEngine();

    // 2.定义关闭iocp
    auto __quitIocpFunc = [this]()->void {
        _iocp->PostQuit();
    };

    // 3.绑定退出iocp
    _quitIocpMonitor = DelegatePlusFactory::Create<decltype(__quitIocpFunc), void>(__quitIocpFunc);

    // 4.预先投递_maxSessionQuantityLimit个accept 快速连入
    char **bufArray = NULL;
    IoDataBase **ioDataArray = NULL;
    NetMethodUtil::PreparePostAccept(_iocp, _acceptorSockFd, bufArray, ioDataArray, _maxSessionQuantityLimit);

    // 5.监听网络连入
    UInt64 quitFlag = static_cast<UInt64>(IocpDefs::IO_QUIT);
    IoDataBase *ioData = NULL;

    auto engine = _engineComp->GetEngine();

    // 等待其他组件ready
    _engineComp->MaskReady(true);
    g_Log->sys<FS_IocpPoller>(_LOGFMT_("iocp acceptor monitor working..."));
    EngineCompsMethods::WaitForAllCompsReady(engine);

    while(pool->IsPoolWorking())
    {
        // 监听iocp
        auto ret = _iocp->WaitForCompletion(*_ioEv);

        // 出错
        if(ret != StatusDefs::Success)
        {
            g_Log->e<FS_IocpPoller>(_LOGFMT_("_AcceptorMonitor.WaitForCompletion error ret[%d]"), ret);
            break;
        }

        // 处理iocp退出
        if(_ioEv->_sessionId == quitFlag)
        {
            g_Log->sys<FS_IocpPoller>(_LOGFMT_("_AcceptorMonitor iocp退出 threadId<%llu> code=%llu")
                                      , SystemUtil::GetCurrentThreadId(), _ioEv->_sessionId);
            break;
        }

        // 接受连接 完成
        ioData = reinterpret_cast<IoDataBase *>(_ioEv->_ioData);
        if(IocpDefs::IO_ACCEPT == ioData->_ioType)
        {
            // TODO:在getacceptAddrInfo时候需要考虑线程是否安全
            sockaddr_in *clientAddrInfo = NULL;
            _iocp->GetClientAddrInfo(ioData->_wsaBuff.buf, clientAddrInfo);

            _HandleSessionWillConnect(netEngine, ioData->_sock, clientAddrInfo);

            // 重新post加速连入
            const auto st = _iocp->PostAccept(_acceptorSockFd, ioData);
            if(st != StatusDefs::Success)
            {
                g_Log->e<FS_IocpPoller>(_LOGFMT_("post accept fail whnen new session create listen sock[%llu], iodata addr[%p] st[%d]")
                                        , _acceptorSockFd, ioData, st);
            }
        }
    }

    _engineComp->MaskReady(false);
    SocketUtil::DestroySocket(_acceptorSockFd);
    _iocp->Destroy();
    NetMethodUtil::FreePrepareAcceptBuffers(bufArray, ioDataArray, _maxSessionQuantityLimit);

    g_Log->sys<FS_IocpPoller>(_LOGFMT_("Acceptor FS_IocpPoller  compId[%u] threadId[%llu] end")
                              , _engineComp->GetCompId(), SystemUtil::GetCurrentThreadId());
}

FS_NAMESPACE_END

#endif
