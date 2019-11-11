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
 * @file  : IFS_ServerConfigMgr.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/07
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/net/Impl/IFS_ServerConfigMgr.h"
#include "base/common/net/Defs/FS_NetDefs.h"
#include "base/common/net/Impl/FS_ServerCore.h"

#include "base/common/status/status.h"
#include "base/common/component/Impl/FS_CpuInfo.h"
#include "base/common/log/Log.h"
#include "base/common/component/Impl/File/FS_IniFile.h"


FS_NAMESPACE_BEGIN

IFS_ServerConfigMgr::IFS_ServerConfigMgr()
    :_ini(NULL)
{
    g_SvrCfg = this;
}

IFS_ServerConfigMgr::~IFS_ServerConfigMgr()
{
    Fs_SafeFree(_ini);
}

Int32 IFS_ServerConfigMgr::Init()
{
    _ini = new FS_IniFile;
    _ini->SetPath(SVR_CONFIG_PATH_NAME);

    BUFFER256 cfgs;
    char *ptr = cfgs;
    if(!_ini->ReadAllKeyValueOfSection(SVR_CFG_LISTENER_SEG, ptr, static_cast<UInt16>(sizeof(cfgs))))
    {
        _InitDefCfgs();
    }

//     FS_CpuInfo cpuInfo;
//     if(!cpuInfo.Initialize())
//     {
//         g_Log->e<IFS_ServerConfigMgr>(_LOGFMT_("cpuInfo.Initialize fail"));
//         return StatusDefs::Failed;
//     }

    return StatusDefs::Success;
}

FS_String IFS_ServerConfigMgr::GetListenIp() const
{
    BUFFER256 outStr = {};
    char *ptr = outStr;
    _ini->ReadStr(SVR_CFG_LISTENER_SEG, SVR_CFG_LISTENER_IP_KEY, "", ptr, sizeof(outStr));
    return outStr;
}

UInt16 IFS_ServerConfigMgr::GetListenPort() const
{
    return static_cast<UInt16>(_ini->ReadInt(SVR_CFG_LISTENER_SEG, SVR_CFG_LISTENER_PORT_KEY, 0));
}

Int32 IFS_ServerConfigMgr::GetMaxConnectQuantityLimit() const
{
    return static_cast<Int32>(_ini->ReadInt(SVR_CFG_TRANSFER_SEG, SVR_CFG_LISTENER_CLN_LIMIT_KEY, 0));
}

Int32 IFS_ServerConfigMgr::GetTransferCnt() const
{
    return static_cast<Int32>(_ini->ReadInt(SVR_CFG_LISTENER_SEG, SVR_CFG_TRANSFER_SEG_CNT_KEY, 0));
}

Int32 IFS_ServerConfigMgr::GetHeartbeatDeadTimeInterval() const
{
    return static_cast<Int32>(_ini->ReadInt(SVR_CFG_TRANSFER_SEG, SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL_KEY, 0));
}

Int32 IFS_ServerConfigMgr::GetDispatcherCnt() const
{
    return static_cast<Int32>(_ini->ReadInt(SVR_CFG_DISPATCHER_SEG, SVR_CFG_DISPATCHER_CNT_KEY, 0));
}

Int32 IFS_ServerConfigMgr::_InitDefCfgs()
{
    #pragma region listener
    // ip
    _ini->WriteStr(SVR_CFG_LISTENER_SEG, SVR_CFG_LISTENER_IP_KEY, SVR_CFG_LISTENER_IP);
    // port
    _ini->WriteStr(SVR_CFG_LISTENER_SEG, SVR_CFG_LISTENER_PORT_KEY, SVR_CFG_LISTENER_PORT);
    // 最大连接数
    _ini->WriteStr(SVR_CFG_LISTENER_SEG, SVR_CFG_LISTENER_CLN_LIMIT_KEY, SVR_CFG_LISTENER_CLN_LIMIT);
    #pragma endregion

    #pragma region transfer
    // 数据传输线程数
    _ini->WriteStr(SVR_CFG_TRANSFER_SEG, SVR_CFG_TRANSFER_SEG_CNT_KEY, SVR_CFG_TRANSFER_SEG_CNT);
    _ini->WriteStr(SVR_CFG_TRANSFER_SEG, SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL_KEY, SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL);
    #pragma endregion

    #pragma region dispatcher
    _ini->WriteStr(SVR_CFG_DISPATCHER_SEG, SVR_CFG_DISPATCHER_CNT_KEY, SVR_CFG_DISPATCHER_CNT);
    #pragma endregion

    // 检查是否写入正确
    BUFFER256 buffer = {};
    char *ptr = buffer;
    _ini->ReadStr(SVR_CFG_LISTENER_SEG, SVR_CFG_LISTENER_IP_KEY, "", ptr, sizeof(buffer));
    if(strcmp(buffer, "127.0.0.1") != 0)
    {
        g_Log->e<IFS_ServerConfigMgr>(_LOGFMT_("_InitDefCfgs fail ip not match"));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    UInt32 port = _ini->ReadInt(SVR_CFG_LISTENER_SEG, SVR_CFG_LISTENER_PORT_KEY, 0);
    if(port != atoi(SVR_CFG_LISTENER_PORT))
    {
        g_Log->e<IFS_ServerConfigMgr>(_LOGFMT_("_InitDefCfgs fail port not match"));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    UInt32 maxConnectQuantity = _ini->ReadInt(SVR_CFG_LISTENER_SEG, SVR_CFG_LISTENER_CLN_LIMIT_KEY, 0);
    if(maxConnectQuantity != atoi(SVR_CFG_LISTENER_CLN_LIMIT))
    {
        g_Log->e<IFS_ServerConfigMgr>(_LOGFMT_("_InitDefCfgs fail maxConnectQuantity not match"));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    UInt32 cnt = _ini->ReadInt(SVR_CFG_TRANSFER_SEG, SVR_CFG_TRANSFER_SEG_CNT_KEY, 0);
    if(cnt != atoi(SVR_CFG_TRANSFER_SEG_CNT))
    {
        g_Log->e<IFS_ServerConfigMgr>(_LOGFMT_("_InitDefCfgs fail transfer cnt not match"));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    UInt32 heartbeatTime = _ini->ReadInt(SVR_CFG_TRANSFER_SEG, SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL_KEY, 0);
    if(heartbeatTime != atoi(SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL))
    {
        g_Log->e<IFS_ServerConfigMgr>(_LOGFMT_("_InitDefCfgs fail SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL[%u] not match SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL[%u] default")
                                      , heartbeatTime, static_cast<UInt32>(atoi(SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL)));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    UInt32 dispatcherCnt = _ini->ReadInt(SVR_CFG_DISPATCHER_SEG, SVR_CFG_DISPATCHER_CNT_KEY, 0);
    if(dispatcherCnt != atoi(SVR_CFG_DISPATCHER_CNT))
    {
        g_Log->e<IFS_ServerConfigMgr>(_LOGFMT_("_InitDefCfgs fail SVR_CFG_DISPATCHER_CNT[%u] not match SVR_CFG_DISPATCHER_CNT[%u] default")
                                      , dispatcherCnt, static_cast<UInt32>(atoi(SVR_CFG_DISPATCHER_CNT)));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    return StatusDefs::Success;
}
FS_NAMESPACE_END
