﻿/*!
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
 * @file  : exportbase.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief :
 * 
 *
 * 
 */
#ifndef __FRIGHTEN_STONE_BASE_EXPORT_BASE_H__
#define __FRIGHTEN_STONE_BASE_EXPORT_BASE_H__

#pragma once

#ifndef __FS_THREAD_SAFE__
#define __FS_THREAD_SAFE__ 0
#endif

//导出定义
#ifndef BASE_EXPORT
    #ifdef  FRIGHTEN_STONE_BASE_EXPORT_BASE_DLL
        #define BASE_EXPORT _declspec(dllexport)
    #else
        #define BASE_EXPORT _declspec(dllimport)
    #endif
#endif

//定义文件名
#ifdef _DEBUG
    #define FRIGHTEN_STONE_BASE_EXPORT_BASE_DLL_NAME                "baseD.dll"    //
#else
    #define FRIGHTEN_STONE_BASE_EXPORT_BASE_DLL_NAME                "base.dll"    //
#endif

#pragma region Warning Disable
#pragma warning(disable:4251)               // 
#define D_SCL_SECURE_NO_WARNINGS            // disable warning C4996
#pragma endregion

#include <base/common/common.h>
#ifndef FRIGHTEN_STONE_BASE_EXPORT_BASE_DLL
    #ifdef _DEBUG
        #pragma comment(lib, "baseD.lib")
    #else
        #pragma comment(lib, "base.lib")
    #endif
#endif

#endif
