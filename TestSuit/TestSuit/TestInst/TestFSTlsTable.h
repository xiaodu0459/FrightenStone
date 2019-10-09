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
 * @file  : TestFSTlsTable.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/9
 * @brief :
 * 
 *
 * 
 */
#ifndef __Test_TestFSTlsTable_H__
#define __Test_TestFSTlsTable_H__
#pragma once

#include "stdafx.h"

class TestTlsTableTask
{
public:
    static void Task(const fs::FS_ThreadPool *pool)
    {
        auto tlsTable = g_ThreadTlsTableMgr->GetAndCreateThisThreadTable();
        auto testElem = tlsTable->GetElement<fs::Tls_TestTls>(fs::TlsElemType::Tls_TestTls);
        if(!testElem)
            testElem = tlsTable->AllocElement<fs::Tls_TestTls>(fs::TlsElemType::Tls_TestTls);

        Int32 &tlsCount = testElem->count;
        Int32 threadId = fs::SystemUtil::GetCurrentThreadId();
        for(Int32 i = 0; i < 10; ++i)
        {
            g_Log->i<TestTlsTableTask>(_LOGFMT_("threadId[%d], tlsCount[%d]"), threadId, tlsCount);
            ++tlsCount;
            Sleep(1000);
        }

        // �ͷű��߳���Դ
        g_ThreadTlsTableMgr->FreeThisThreadTable();

        // �߳̽���
        g_Log->i<TestTlsTableTask>(_LOGFMT_("TestTlsTableTask end"));
    }
};

class TestFSTlsTable
{
public:
    static void Run()
    {
        g_Log->InitModule(NULL);
        fs::FS_ThreadPool pool(0, 10);
        for(Int32 i = 0; i < 10; ++i)
        {
            auto testTlsTask = fs::DelegatePlusFactory::Create(&TestTlsTableTask::Task);
            pool.AddTask(testTlsTask, true);
        }
        getchar();
    }
};

#endif
