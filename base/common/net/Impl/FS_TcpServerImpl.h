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
 * @file  : FS_TcpServerImpl.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/08/03
 * @brief :
 * 
 *
 * 
 */
#ifdef __Base_Common_Net_Impl_FS_TcpServer_H__
#pragma once

FS_NAMESPACE_BEGIN

template<class ServerT>
void FS_TcpServer::Start(Int32 svrQuantity)
{
    // ���������������ÿ��������һ���̣߳�
    for(int n = 0; n < svrQuantity; ++n)
    {
        auto ser = new ServerT();
        ser->SetId(n + 1);
        ser->SetClientNum((_maxClient / svrQuantity) + 1);
        _fsServers.push_back(ser);
        // ע�������¼����ܶ���
        ser->SetEventObj(this);
        // ������Ϣ�����߳�
        ser->Start();
    }

    // ���������������
    auto monitorDelegate = DelegatePlusFactory::Create(this, &FS_TcpServer::OnNetMonitorTask);
    _threadPool->AddTask(monitorDelegate);
}

#pragma region misc
inline SOCKET FS_TcpServer::_GetSocket()
{
    return _sock;
}
#pragma  endregion

FS_NAMESPACE_END

#endif