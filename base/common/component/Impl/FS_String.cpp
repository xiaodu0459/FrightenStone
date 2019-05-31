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
 * @file  : FS_String.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/component/Impl/FS_String.h"
#include "base/common/basedefs/Macro/MacroDefs.h"
#include <algorithm>
#include <stdarg.h>
#include <set>

inline std::ostream &operator <<(std::ostream &stream, const fs::FS_String &str)
{
    return stream << str.GetRaw().c_str();
}

FS_NAMESPACE_BEGIN

#ifdef _WIN32
const char *FS_String::endl = "\r\n";
#else
const char *FS_String::endl = "\n";
#endif

FS_String::FS_String()
{
    _cache[0] = 0;
}
FS_String::~FS_String()
{
    
}

FS_String::FS_String(char other)
{
    _buffer = other;
    _cache[0] = 0;
}

FS_String::FS_String(const std::string &other)
    :_buffer(other)
{
    // _buffer = other;
    _cache[0] = 0;
}

FS_String::FS_String(const char *other)
{
    _buffer = other;
    _cache[0] = 0;
}

FS_String FS_String::ToHexString() const
{
    FS_String info;
    char cache[4] = {0};
    const Int64 bufferSize = static_cast<Int64>(_buffer.size());
    for(Int64 i = 0; i < bufferSize; ++i)
    {
        cache[0] = 0;
        const Int32 len = sprintf_s(cache, sizeof(cache), "%02x", static_cast<U8>(_buffer[i]));
        cache[len] = 0;
        info << cache;
    }

    return info;
}

FS_String::_These FS_String::Split(char sep, size_type max_split) const
{
    return this->Split(FS_String(sep), max_split);
}

FS_String::_These FS_String::Split(const char *sep, size_type max_split) const
{
    return this->Split(FS_String(sep), max_split);
}

FS_String::_These FS_String::Split(const FS_String &sep, std::string::size_type max_split, bool onlyLikely) const
{
    _These substrs;
    if(sep.empty() || max_split == 0 || this->empty())
    {
        substrs.push_back(*this);
        return substrs;
    }

    size_type idx = 0;
    UInt32 splitTimes = 0;
    for(; splitTimes < static_cast<UInt32>(max_split); ++splitTimes)
    {
        size_type findIdx = std::string::npos;
        if(onlyLikely)
        {
            for(size_t i = 0; i < sep.size(); i++)
            {
                findIdx = _buffer.find(sep._buffer[i], idx);
                if(findIdx != std::string::npos)
                    break;
            }
        }
        else
        {
            findIdx = _buffer.find(sep._buffer, idx);
        }

        if(findIdx == std::string::npos)
            break;

        substrs.push_back(_buffer.substr(idx, findIdx - idx));
        if((idx = findIdx + 1) == this->size())
        {
            substrs.push_back(FS_String());
            break;
        }
    }

    if(idx != this->size())
        substrs.push_back(_buffer.substr(idx));

    return substrs;
}

FS_String::_These FS_String::Split(const FS_String::_These &seps, FS_String::size_type max_split) const
{
    _These substrs;
    if(seps.empty() || max_split == 0 || this->empty())
    {
        substrs.push_back(*this);
        return substrs;
    }

    size_type idx = 0;
    UInt32 splitTimes = 0;
    std::set<size_type> minIdx;
    for(; splitTimes < static_cast<UInt32>(max_split); ++splitTimes)
    {
        size_type findIdx = std::string::npos;
        minIdx.clear();
        for(size_t i = 0; i < seps.size(); i++)
        {
            findIdx = _buffer.find(seps[i]._buffer, idx);
            if(findIdx != std::string::npos)
                minIdx.insert(findIdx);
        }

        if(!minIdx.empty())
            findIdx = *minIdx.begin();

        if(findIdx == std::string::npos)
            break;

        substrs.push_back(_buffer.substr(idx, findIdx - idx));
        if((idx = findIdx + 1) == this->size())
        {
            substrs.push_back(FS_String());
            break;
        }
    }

    if(idx != this->size())
        substrs.push_back(_buffer.substr(idx));

    return substrs;
}

void FS_String::add_utf8_bomb()
{
    if(!has_utf8_bomb())
        _buffer.insert(0, reinterpret_cast<const char *>("\xef\xbb\xbf"));
}

std::string::size_type FS_String::length_with_utf8() const
{
    std::string::size_type count = 0;
    std::string::size_type bytePos = 0;
    while((bytePos = _next_utf8_char_pos(bytePos)) != std::string::npos)
        count++;

    return count;
}

std::string FS_String::substr_with_utf8(std::string::size_type pos, std::string::size_type n) const
{
    std::string::size_type utf8Len = length_with_utf8();
    if(pos >= utf8Len || n == 0)
        return std::string();

    _These substrs;
    split_utf8_string(static_cast<Long>(pos), substrs);
    if(substrs.empty())
        return std::string();

    FS_String str1 = *substrs.rbegin();
    utf8Len = str1.length_with_utf8();
    pos = (n == std::string::npos || n > utf8Len) ? utf8Len : n;

    substrs.clear();
    str1.split_utf8_string(static_cast<Long>(pos), substrs);
    if(substrs.empty())
        return std::string();

    return substrs[0]._buffer;
}

void FS_String::split_utf8_string(Long charIndex, _These &strs) const
{
    strs.clear();
     if(charIndex == 0)
    {
        strs.push_back(*this);
        return;
    }

    std::string::size_type utf8Count = length_with_utf8();
    if(UNLIKELY(utf8Count == std::string::npos))
    {
        strs.push_back(*this);
        return;
    }

    charIndex = (charIndex < 0) ?
        static_cast<Long>(utf8Count) + charIndex : charIndex;
    if(charIndex <= 0 || charIndex >= static_cast<Long>(utf8Count))
    {
        strs.push_back(*this);
        return;
    }

    std::string::size_type bytePos = 0;
    std::string::size_type charPos = 0;
    while(static_cast<Long>(charPos) != charIndex)
    {
        bytePos = _next_utf8_char_pos(bytePos);
        charPos++;
    }

    strs.push_back(_buffer.substr(0, bytePos));
    strs.push_back(_buffer.substr(bytePos));
}

void FS_String::scatter_utf8_string(_These &chars, std::string::size_type scatterCount) const
{
    chars.clear();

    if(scatterCount == 0)
        scatterCount = std::string::npos;
    else if(scatterCount != std::string::npos)
        scatterCount -= 1;

    if(scatterCount == 0)
    {
        chars.push_back(*this);
        return;
    }

    std::string::size_type curPos = 0;
    std::string::size_type prevPos = 0;
    std::string::size_type curScatterCount = 0;
    while((curPos = _next_utf8_char_pos(prevPos)) != std::string::npos)
    {
        chars.push_back(_buffer.substr(prevPos, curPos - prevPos));

        if(scatterCount != std::string::npos && ++curScatterCount >= scatterCount)
        {
            chars.push_back(_buffer.substr(curPos));
            break;
        }

        prevPos = curPos;
    }
}

bool FS_String::has_utf8_bomb() const
{
    if(_buffer.size() < 3)
        return false;

    return (::memcmp(reinterpret_cast<const char *>(_buffer.data()),
                     reinterpret_cast<const char *>("\xef\xbb\xbf"), 3) == 0) ? true : false;
}

void FS_String::remove_utf8_bomb()
{
    if(has_utf8_bomb())
        _buffer.erase(0, 3);
}

std::string::size_type FS_String::_next_utf8_char_pos(std::string::size_type &beginBytePos) const
{
    if(beginBytePos == 0 && has_utf8_bomb())
        beginBytePos += 3;

    if(beginBytePos == std::string::npos || beginBytePos >= _buffer.size())
        return std::string::npos;

    std::string::size_type waitCheckCount = std::string::npos;

    // 0xxx xxxx
    // Encoding len: 1 byte.
    U8 ch = static_cast<U8>(_buffer.at(beginBytePos));
    if((ch & 0x80) == 0x00)
        waitCheckCount = 0;
    // 110x xxxx
    // Encoding len: 2 bytes.
    else if((ch & 0xe0) == 0xc0)
        waitCheckCount = 1;
    // 1110 xxxx
    // Encoding len: 3 bytes.
    else if((ch & 0xf0) == 0xe0)
        waitCheckCount = 2;
    // 1111 0xxx
    // Encoding len: 4 bytes.
    else if((ch & 0xf8) == 0xf0)
        waitCheckCount = 3;
    // 1111 10xx
    // Encoding len: 5 bytes.
    else if((ch & 0xfc) == 0xf8)
        waitCheckCount = 4;
    // 1111 110x
    // Encoding len: 6 bytes.
    else if((ch & 0xfe) == 0xfc)
        waitCheckCount = 5;

    if(waitCheckCount == std::string::npos)
        return std::string::npos;

    std::string::size_type curPos = beginBytePos + 1;
    std::string::size_type endPos = curPos + waitCheckCount;
    if(endPos > _buffer.size())
        return std::string::npos;

    for(; curPos != endPos; curPos++)
    {
        ch = static_cast<U8>(_buffer.at(curPos));
        if((ch & 0xc0) != 0x80)
            return std::string::npos;
    }

    return endPos;
}

FS_String &FS_String::FormatCStyle(const char *fmt, ...)
{
    char *buf; int len;
    __FS_BuildFormatStr_(fmt, buf, len);
    _buffer.append(buf, len);
    FsSafeDelete(buf);

    return *this;
}

UInt64 FS_String::CalcPlaceHolderNum(const std::string &fmtStr)
{
    UInt64 startPos = 0, nextPos = 0, cnt = 0;

    for(startPos = 0;
        startPos = fmtStr.find_first_of('%', startPos),
        ((startPos != std::string::npos) ? ((nextPos = fmtStr.find_first_of('%', startPos + 1)), true) : false); )
    {
        if(nextPos != std::string::npos)
        {
            UInt64 diff = nextPos >= startPos ? (nextPos - startPos) : 0UL;
            if(LIKELY(diff > 1))
            {
                cnt += 2;
                startPos = nextPos + 1;
            }
            else
            {
                startPos = nextPos + 1;
                continue;
            }
        }
        else
        {
            ++cnt;
            break;
        }
    }

    return cnt;
}

bool FS_String::AppendFirstValidPlaceHolderString(const std::string &fmt, UInt64 &firstIndex, std::string &outStr)
{
    if(UNLIKELY(fmt.length() <= 0))
        return false;

    firstIndex = 0;
    firstIndex = fmt.find_first_of('%', firstIndex);
    if(firstIndex == std::string::npos)
    {
        outStr += fmt.substr(0);
        return true;
    }

    if(firstIndex != 0)
        outStr += fmt.substr(0, firstIndex);

    return true;
}

std::string FS_String::NextPlaceHolderPos(const std::string &strFmt, UInt64 startIndex, UInt64 &outIndex)
{
    if( UNLIKELY((strFmt.length() <= 0) || 
        (startIndex == std::string::npos)) )
    {
        outIndex = std::string::npos;
        return std::string();
    }

    auto cacheStartIndex = startIndex;
    while((outIndex = strFmt.find_first_of('%', cacheStartIndex + 1), true))
    {
        if(outIndex == std::string::npos)
            return strFmt.substr(startIndex);

        UInt64 diff = outIndex >= cacheStartIndex ? (outIndex - cacheStartIndex) : 0;
        if(diff > 1)
            return strFmt.substr(startIndex, outIndex - startIndex);

        ++cacheStartIndex;
    }

    outIndex = std::string::npos;
    return std::string();
}

void FS_String::_append_fmt_str_cstyle(const Byte8 *fmtstr, ...)
{
    char *buf = NULL;
    Int32 len = 0;
    __FS_BuildFormatStr_(fmtstr, buf, len);
    _buffer += buf;
    Fs_SafeFree(buf);
}

FS_NAMESPACE_END
