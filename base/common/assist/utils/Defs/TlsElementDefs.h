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
 * @file  : TlsElementDefs.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/9
 * @brief :
 * 
 *
 * 
 */

#ifndef __Base_Common_Assist_Utils_Impl_Defs_TlsElementDefs_H__
#define __Base_Common_Assist_Utils_Impl_Defs_TlsElementDefs_H__
#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/assist/utils/Defs/ITlsBase.h"
#include "base/common/objpool/objpool.h"

// 类型识别缓冲大小
#ifndef __FS_RTTI_BUF_SIZE__
#define __FS_RTTI_BUF_SIZE__    512
#endif

FS_NAMESPACE_BEGIN

// 所有局部存储对象请派生于ITlsBase
// 线程局部存储需要存储的对象类型
class BASE_EXPORT TlsElemType
{
public:
    enum
    {
        Begin = 0,
        Tls_Rtti,           // 类型识别
        Tls_TestTls,        // 测试tls
        End,
    };
};

// 类型识别线程局部存储
struct BASE_EXPORT Tls_Rtti : public ITlsBase
{
    OBJ_POOL_CREATE_DEF(Tls_Rtti);

    Tls_Rtti();
    virtual ~Tls_Rtti();
    virtual void Release();

    char rtti[__FS_RTTI_BUF_SIZE__];
};

// 测试线程局部存储
struct BASE_EXPORT Tls_TestTls : public ITlsBase
{
    OBJ_POOL_CREATE_DEF(Tls_TestTls);

    Tls_TestTls();
    virtual ~Tls_TestTls();

    Int32 count;
};

FS_NAMESPACE_END

#include "base/common/assist/utils/Defs/TlsElementDefsImpl.h"

#endif
