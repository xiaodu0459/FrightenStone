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
 * @file  : FS_EpollMsgDispatcherImpl.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2020/01/26
 * @brief :
 */

#ifdef __Frame_Include_FrightenStone_Common_Net_Impl_FS_EpollMsgDispatcher_H__
#pragma once

FS_NAMESPACE_BEGIN

inline void FS_EpollMsgDispatcher::BindBusinessLogic(IFS_BusinessLogic *businessLogic)
{
    _logic = businessLogic;
}

inline Int32 FS_EpollMsgDispatcher::GetSessionCnt() const
{
    return _curSessionQuatity;
}

inline TimeWheel *FS_EpollMsgDispatcher::GetTimeWheel()
{
    return _timeWheel;
}

inline const IFS_Session *FS_EpollMsgDispatcher::GetSession(UInt64 sessionId) const
{
    return reinterpret_cast<const IFS_Session *>(_GetSession(sessionId));
}

inline void FS_EpollMsgDispatcher::_RemoveFromPostRecvQueue(FS_EpollSession *session)
{
    _toPostRecv.erase(session);
}

inline void FS_EpollMsgDispatcher::_RemoveFromPostSendQueue(FS_EpollSession *session)
{
    _toPostSend.erase(session);
}

inline FS_EpollSession *FS_EpollMsgDispatcher::_GetSession(UInt64 sessionId)
{
    auto iterSession = _sessions.find(sessionId);
    if(iterSession == _sessions.end())
        return NULL;

    return iterSession->second;
}

inline const FS_EpollSession *FS_EpollMsgDispatcher::_GetSession(UInt64 sessionId) const
{
    auto iterSession = _sessions.find(sessionId);
    if(iterSession == _sessions.end())
        return NULL;

    return iterSession->second;
}

FS_NAMESPACE_END

#endif
