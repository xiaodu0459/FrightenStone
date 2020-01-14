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
 * @file  : GlobalSysMgr.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2020/1/14
 * @brief :
 */

#pragma once

#include "FrightenStone/exportbase.h"
#include "TestInst/TestServer/GlobalSys/Impl/IGlobalSysMgr.h"

class GlobalSysMgr : public IGlobalSysMgr
{
public:
    GlobalSysMgr();
    virtual ~GlobalSysMgr();

public:
    virtual Int32 RegisterGlobalSys(fs::IFS_LogicSysFactory *sysFactory);

public:
    /* 主线程执行 */
    virtual Int32 OnInitialize();
    virtual Int32 BeforeStart();
    virtual Int32 Start();
    ///////////////////////////////////////

    /* 业务线程执行 */ 
    virtual void BeforeClose();
    virtual void Close();
    ///////////////////////////////

public:
    virtual const fs::ILogicSysInfo *GetSysInfo(const fs::FS_String &globalSysName) const;
    virtual const std::vector<fs::ILogicSysInfo *> &GetSysInfosList() const;
    virtual const std::map<fs::FS_String, fs::ILogicSysInfo *> &GetSysInfosDict() const;
    virtual fs::IGlobalSys *GetSys(const fs::FS_String &sysName);

private:
    void _Cleanup();

private:
    std::vector<fs::ILogicSysInfo *> _sysInfosList;
    std::map<fs::FS_String, fs::ILogicSysInfo *> _sysInfosDict;

    std::map<fs::FS_String, fs::IGlobalSys *> _globalSysNameRefGlobalSyss;
    std::map<fs::FS_String, fs::IGlobalSys *> _globalSysNameRefIGlobalSyss; // 带"I"的global系统基类接口
};