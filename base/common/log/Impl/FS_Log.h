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
 * @file  : FS_Log.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/6/12
 * @brief :
 *          1.提供写日志接口（传入任意参数）
 *          2.分目录日志 crash, system, netlayerlog（协议信息）业务层日志等
 *          3.按照文件大小分文件
 *          4.按照问题的优先级分:warning,error,debug,info等
 *          5.支持json log 风格：
 *                              {
                                    time:(精确到微妙)fmt:1970-01-01 00:00:00.123456
                                    class name:
                                    function:
                                    line:
                                    level:
                                    content:
                                    {
                                        op:
                                        status:statusCode
                                        ecxceptioninfo:
                                        stackinfo:
                                    }
                                }
 */
#ifndef __Base_Common_Log_Impl_Log_H__
#define __Base_Common_Log_Impl_Log_H__
#pragma once

#include "base/common/log/Interface/ILog.h"
#include "base/common/log/Defs/LogData.h"
#include "base/common/component/Impl/FS_Delegate.h"
#include "base/common/asyn/asyn.h"

FS_NAMESPACE_BEGIN

class FS_ThreadPool;
class LogFile;
class BASE_EXPORT FS_Log : public ILog
{
public:
    FS_Log(const Byte8 *processName);
    virtual ~FS_Log();

    /* 日志hook */
    virtual void InstallLogHookFunc(Int32 level, IDelegatePlus<void, const LogData *> *delegate);    // 抽象的delegate
    void UnInstallLogHook(Int32 level);

    /* 功能函数 */
    Int32 InitModule();
    virtual void FinishModule();
    Int32 AddLogFile(Int32 fileUnqueIndex, const char *logPath, const char *fileName);
    
protected:
    virtual LogData *_BuildLogData(const Byte8 *className, const Byte8 *funcName, const FS_String &content, Int32 codeLine, Int32 logLevel);
    virtual void _WriteLog(Int32 fileUniqueIndex, LogData *logData);

    std::list<LogData *> *_GetLogDataList(Int32 fileIndex);
    std::list<LogData *> *_NewLogDataList(Int32 fileIndex);

    // 线程操作
private:
    void _OnThreadWriteLog();

private:
    FS_ThreadPool *_threadPool;                                                 // 线程池
    FS_String _processName;                                                     // 进程名
    std::map<Int32, IDelegatePlus<void, const LogData *> *> _levelRefHook;      // 日志级别对应的hook
    Int32 _threadWorkIntervalMsTime;                                            // 日志线程工作间隔时间

    /* 日志文件内容 */
    ConditionLocker _locker;                                                // 锁
    std::map<Int32, LogFile *> _fileUniqueIndexRefLogFiles;                 // 日志id日志文件
    std::map<Int32, std::list<LogData *> *> _fileUniqueIndexRefLogDatas;    // 日志id日志内容
    IDelegatePlus<void> *_threadWriteLogDelegate;                           // 日志线程写日志委托

};

FS_NAMESPACE_END

#include "base/common/log/Impl/FS_LogImpl.h"

#endif
