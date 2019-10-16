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
 * @file  : FS_Packet.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/08/31
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Net_Defs_FS_Packet2_H__
#define __Base_Common_Net_Defs_FS_Packet2_H__
#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/objpool/objpool.h"
#include "base/common/net/protocol/protocol.h"
#include "base/common/memorypool/memorypool.h"
#include "base/common/net/Defs/IocpDefs.h"
#include "base/common/component/Impl/FS_Delegate.h"
#include "base/common/socket/socket.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT FS_Packet2
{
    OBJ_POOL_CREATE_DEF(FS_Packet2);

public:
    FS_Packet2(UInt64 sessionId);
    ~FS_Packet2();

public:
    void Pop(Int32 len);
    void FromMsg(UInt64 ownerId, SOCKET socket, NetMsg_DataHeader *header);
    void FromMsg(NetMsg_DataHeader *header);
    void FromMsg(const char *data, Int32 len);
    void FromMsg(UInt64 ownerId, SOCKET socket, const char *data, Int32 len);
    NetMsg_DataHeader *CastToMsg();
    template<typename ObjType>
    ObjType *CastToMsg();
    char *GiveupBuffer(Int32 &packetSize, Int32 &lastPos);
    void Clear();

    IoDataBase *MakeRecvIoData();
    IoDataBase *MakeSendIoData();

    bool IsFullPacket();
    bool IsEmpty();
    bool HasMsg() const;
    bool NeedWrite() const;
    UInt64 GetSessionId() const;
    SOCKET GetSocket() const;

private:
    void _FromMsg(const char *data, Int32 len);
    void _Clear();

private:
    void _OnSendSucCallback(size_t transferBytes);
    void _OnRecvSucCallback(size_t transferBytes);

private:
    Int32 _packetSize;
    UInt64 _sessionId;      // FS_Packet���������ڲ����
    SOCKET _socket;         // FS_Packet���������ڲ����
    char *_buff;
    Int32 _lastPos;
    IoDataBase _ioData = {};
};

FS_NAMESPACE_END

#include "base/common/net/Defs/FS_Packet2Impl.h"

#endif