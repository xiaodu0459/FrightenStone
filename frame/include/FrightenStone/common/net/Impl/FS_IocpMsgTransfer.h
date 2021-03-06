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
 * @file  : FS_IocpMsgTransfer.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/07
 * @brief :
 * 
 *
 * 
 */
#ifndef __Frame_Include_FrightenStone_Common_Net_Impl_FS_IocpMsgTransfer_H__
#define __Frame_Include_FrightenStone_Common_Net_Impl_FS_IocpMsgTransfer_H__
#pragma once

#ifdef _WIN32

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/net/Impl/IFS_MsgTransfer.h"
#include "FrightenStone/common/asyn/asyn.h"
#include "FrightenStone/common/component/Impl/FS_Delegate.h"
#include "FrightenStone/common/net/Impl/IFS_Session.h"
#include "FrightenStone/common/net/Impl/FS_IocpSession.h"
#include "FrightenStone/common/net/Defs/HeartBeatComp.h"
#include "FrightenStone/common/net/Defs/NetCfgDefs.h"
#include "FrightenStone/common/objpool/objpool.h"

FS_NAMESPACE_BEGIN

struct BriefSessionInfo;
class IFS_NetEngine;
class FS_IocpPoller;

class BASE_EXPORT FS_IocpMsgTransfer : public IFS_MsgTransfer
{
    OBJ_POOL_CREATE_DEF(FS_IocpMsgTransfer);
public:
    FS_IocpMsgTransfer(UInt32 compId, IFS_NetEngine *netEngine);
    virtual ~FS_IocpMsgTransfer();

public:
    virtual Int32 BeforeStart(const NetEngineTotalCfgs &totalCfgs);
    virtual Int32 Start();
    virtual void AfterStart();
    virtual void WillClose();
    virtual void BeforeClose();
    virtual void Close();
    virtual void AfterClose();

    // 网络事件
public:
    virtual bool OnWillConnect(BriefSessionInfo *newSessionInfo);
    virtual void OnPostConnectOpFinish();
    virtual void OnSessionDisconnected(UInt64 sessionId);

public:
    virtual Int32 GetSessionCnt();

private:
    std::atomic_bool _isInit;
     TransferCfgs *_cfgs;
     std::atomic<Int32> _sessionCnt;             // transfer处理的会话个数
     FS_IocpPoller *_poller;

//     _CrtMemState s1;
//     _CrtMemState s2;
//     _CrtMemState s3;
};

FS_NAMESPACE_END

#include "FrightenStone/common/net/Impl/FS_IocpMsgTransferImpl.h"

#endif

#endif
