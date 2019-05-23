#ifndef __Base_Common_Component_Impl_File_FS_File__H__
#define __Base_Common_Component_Impl_File_FS_File__H__
/**
* @file    FS_File.h
* @author  Huiya Song<120453674@qq.com>
* @date    2019/05/20
* @brief
*/

#pragma once

#include <base/exportbase.h>
#include "base/common/basedefs/Macro/MacroDefs.h"
//#include "base/common/component/Impl/Time.h"
//#include "base/common/component/Impl/FS_String.h"
#include "base/common/asyn/Lock/Lock.h"
// #include <string>

FS_NAMESPACE_BEGIN

class Time;
class FS_String;

// 若需要考虑线程安全请手动使用锁
class BASE_EXPORT FS_File
{
public:
    FS_File();
    virtual ~FS_File();

    //打开，读，写，刷新，关闭
public:
    virtual bool Open(const char *fileName, bool isCreate = false, const char *openMode = "ab+", bool useTimestampTailer = false);
    virtual bool Reopen();
    virtual bool Flush();
    virtual bool Close();

    virtual Int64 Write(const void *buffer, UInt64 writeDataLen);
    virtual Int64 Read(FS_String &outBuffer);
    virtual Int64 Read(Int64 sizeLimit, FS_String &outBuffer);
    virtual Int64 ReadOneLine(FS_String &outBuffer);

    void Lock();
    void UnLock();

    bool IsOpen() const;
    const char *GetPath() const;
    const char *GetFileName() const;

public:
    operator bool() const;
    operator FILE *();
    operator const FILE *() const;

protected:
    FILE *_fp = NULL;
    Time _createFileTime;
    Time _modifyFileTime;
    bool _useTimestampTailer = false;
    FS_String _path;
    FS_String _fileName;
    FS_String _extensionName;       // 扩展名
    FS_String _openMode = "ab+";
    Locker  _locker;
};

#pragma region Inline

inline void FS_File::Lock()
{
    _locker.Lock();
}

inline void FS_File::UnLock()
{
    _locker.Unlock();
}

inline bool FS_File::IsOpen() const
{
    return _fp != NULL;
}

inline const char *FS_File::GetPath() const
{
    return _path.c_str();
}

inline const char *FS_File::GetFileName() const
{
    return _fileName.c_str();
}

inline FS_File::operator bool() const
{
    return _fp != NULL;
}

inline FS_File::operator FILE *()
{
    return _fp;
}

inline FS_File::operator const FILE *() const
{
    return _fp;
}
#pragma endregion

FS_NAMESPACE_END

#endif
