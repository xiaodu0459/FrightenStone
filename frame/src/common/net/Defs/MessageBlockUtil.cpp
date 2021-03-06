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
 * @file  : MessageBlockUtil.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/12/29
 * @brief :
 */
#include "stdafx.h"
#include "FrightenStone/common/net/Defs/MessageBlockUtil.h"
#include "FrightenStone/common/net/Defs/FS_NetMessageBlock.h"
#include "FrightenStone/common/net/Defs/IocpDefs.h"
#include "FrightenStone/common/net/Defs/BriefSessionInfo.h"
#include "FrightenStone/common/net/Defs/FS_EpollEvMessageBlock.h"

#include "FrightenStone/common/component/Impl/MessageQueue/MessageQueue.h"
#include "FrightenStone/common/memorypool/memorypool.h"

FS_NAMESPACE_BEGIN

FS_MessageBlock *MessageBlockUtil::BuildTransferMonitorArrivedMessageBlock(UInt32 compId, UInt32 generatorId, IoEvent *ev, Int32 errorCode)
{
    FS_NetArrivedMsg *block = new FS_NetArrivedMsg();
    block->_compId = compId;
    block->_generatorId = generatorId;
    g_MemoryPool->Lock();
    block->_ioEv = g_MemoryPool->Alloc<IoEvent>(sizeof(IoEvent));
    g_MemoryPool->Unlock();
    *block->_ioEv = *ev;
    block->_errorCode = errorCode;
    return block;
}

FS_MessageBlock *MessageBlockUtil::BuildTransferWillConnectMessageBlock(UInt32 compId
                                                                        , UInt32 generatorId
                                                                        , IFS_NetModule *netModule
                                                                        , BriefSessionInfo *newSessionInfo)
{
    FS_NetSessionWillConnectMsg *block = new FS_NetSessionWillConnectMsg;
    block->_compId = compId;
    block->_acceptorCompId = newSessionInfo->_acceptorCompId;
    block->_generatorId = generatorId;
    block->_sessionId = newSessionInfo->_sessionId;
    block->_sock = newSessionInfo->_sock;
    block->_addrInfo = *newSessionInfo->_addrInfo;
    block->_onUserDisconnectedRes = newSessionInfo->_userDisconnectedRes;
    newSessionInfo->_userDisconnectedRes = NULL;
    block->_netModule = netModule;
    return block;
}

FS_MessageBlock * MessageBlockUtil::BuildConnectorPostConnectOpFinish(UInt32 compId, UInt32 generatorId)
{
#ifdef _WIN32
    auto block = new FS_NetMsgBufferBlock();
    block->_compId = compId;
    block->_generatorId = generatorId;
    block->_netMessageBlockType = NetMessageBlockType::Net_NetConnectorConnectOpFinish;
    return block;
#else
    auto block = new FS_EpollEvMessageBlock();
    block->_compId = compId;
    block->_generatorId = generatorId;
    block->_messageType = FS_EpollEvMessageType::ConnectorConnectOpFinish;
    return block;
#endif
}

#ifndef _WIN32

FS_MessageBlock * MessageBlockUtil::BuildTransferEpollEvSessionWillConnectMsg(UInt32 compId, UInt32 generatorId, FS_EpollPoller *poller, BriefSessionInfo *newSessionInfo)
{
    EpollEvSessionWillConnectMsg *block = new EpollEvSessionWillConnectMsg;
    block->_compId = compId;
    block->_acceptorCompId = newSessionInfo->_acceptorCompId;
    block->_generatorId = generatorId;
    block->_sessionId = newSessionInfo->_sessionId;
    block->_sock = newSessionInfo->_sock;
    block->_addrInfo = *newSessionInfo->_addrInfo;
    block->_onUserDisconnectedRes = newSessionInfo->_userDisconnectedRes;
    newSessionInfo->_userDisconnectedRes = NULL;
    block->_poller = poller;
    return block;
}

FS_MessageBlock * MessageBlockUtil::BuildEpollInEvMessageBlock(UInt64 sessionId)
{
    EpollEvEpollInEvMessageBlock *block = new EpollEvEpollInEvMessageBlock;
    block->_sessionId = sessionId;
    return block;
}

FS_MessageBlock *MessageBlockUtil::BuildEpollOutEvMessageBlock(UInt64 sessionId)
{
    EpollEvEpollOutEvMessageBlock *block = new EpollEvEpollOutEvMessageBlock;
    block->_sessionId = sessionId;
    return block;
}
#endif

FS_NAMESPACE_END

