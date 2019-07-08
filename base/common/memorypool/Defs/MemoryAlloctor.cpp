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
 * @file  : MemoryAlloctor.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/7/5
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/memorypool/Defs/MemoryAlloctor.h"
#include "base/common/memorypool/Defs/MemoryBlock.h"
#include "base/common/log/Log.h"

#define BLOCK_AMOUNT_DEF    1024    // Ĭ���ڴ������

FS_NAMESPACE_BEGIN

IMemoryAlloctor::IMemoryAlloctor()
    :_buf(NULL)
    ,_blockAmount(BLOCK_AMOUNT_DEF)
    ,_usableBlockHeader(NULL)
    ,_blockSize(0)
{
    
}

IMemoryAlloctor::~IMemoryAlloctor()
{
    _FinishMemory();
}

void *IMemoryAlloctor::AllocMemory(size_t bytesCnt, const FS_String &objName)
{
//     if(UNLIKELY(!_buf))
//         _InitMemory();

    MemoryBlock *newBlock = NULL;

    // �ڴ治����ʹ��ϵͳ�ڴ���䷽��
    if (_usableBlockHeader == 0)
    {
        newBlock = reinterpret_cast<MemoryBlock *>(::malloc(bytesCnt + sizeof(MemoryBlock)));
        newBlock->_isInPool = false;    // �����ڴ����
        newBlock->_ref = 1;             // ��1������
        newBlock->_alloctor = this;     // ������
        newBlock->_nextBlock = 0;
        auto len = sprintf(newBlock->_objName, "%s", objName.c_str());
        if(len > 0)
            newBlock->_objName[BUFFER_LEN256 > len ? len : BUFFER_LEN256 - 1] = 0;
        else
            newBlock->_objName[0] = 0;
    }
    else
    {
        /**
        *   get one node from free list
        */
        newBlock = _usableBlockHeader;
        _usableBlockHeader = _usableBlockHeader->_nextBlock;
        newBlock->_alloctor =   this;
        newBlock->_isInPool = true;
        auto len = sprintf(newBlock->_objName, "%s", objName.c_str());
        if(len > 0)
            newBlock->_objName[BUFFER_LEN256 > len ? len : BUFFER_LEN256 - 1] = 0;
        else
            newBlock->_objName[0] = 0;

        ASSERT(newBlock->_ref == 0);
        newBlock->_ref = 1;
    }
    return ((char*)newBlock) + sizeof(MemoryBlock);   // ��block����һ����ַ��ʼ�������������뵽���ڴ�
}

void IMemoryAlloctor::FreeMemory(void *ptr)
{
    // �ڴ��ͷ
    char *ptrToFree = reinterpret_cast<char*>(ptr);
    MemoryBlock *blockHeader = reinterpret_cast<MemoryBlock*>(reinterpret_cast<char*>(ptrToFree - sizeof(MemoryBlock)));

    // ���ü���
    if(--blockHeader->_ref != 0)
        return;

    // �����ڴ���������ü���Ϊ0
    if (!blockHeader->_isInPool)
    {
        ::free(blockHeader);
        blockHeader->_objName[0] = 0;
        return;
    }

    // ���ͷŵĽڵ�嵽����ͷ�ڵ�֮ǰ
    blockHeader->_objName[0] = 0;
    blockHeader->_nextBlock = _usableBlockHeader;
    _usableBlockHeader = blockHeader;
}

void IMemoryAlloctor::_InitMemory()
{
    ASSERT(_buf == 0);
    if(!_buf)
        return;

    /**
    *   ������Ҫ������ڴ��С�����а����ڴ�ͷ���ݴ�С
    */
    size_t	bufSize = _blockSize * _blockAmount;

    /**
    *   �����ڴ�
    */
    _buf = reinterpret_cast<char*>(::malloc(bufSize));
    memset(_buf, 0, bufSize);

    /**
    *   ����ͷ��β��ָ��ͬһλ��
    */
    _usableBlockHeader = reinterpret_cast<MemoryBlock*>(_buf);
    _usableBlockHeader->_ref = 0;
    _usableBlockHeader->_alloctor = this;
    _usableBlockHeader->_nextBlock = 0;
    _usableBlockHeader->_isInPool = true;
    _usableBlockHeader->_objName[0] = 0;
    MemoryBlock *temp = _usableBlockHeader;

    // �����ڴ������
    for(size_t i = 1; i < _blockAmount; ++i)
    {
        char *cache = (_buf + _blockSize * i);
        MemoryBlock *block = reinterpret_cast<MemoryBlock*>(cache);
        block->_ref = 0;
        block->_isInPool = true;
        block->_alloctor = this;
        block->_nextBlock = 0;
        block->_objName[0] = 0;
        temp->_nextBlock = block;
        temp = block;
    }
}

void IMemoryAlloctor::_FinishMemory()
{
    // �ռ��ڴ�й©
    std::map<FS_String, Int64> objNameRefAmount;
    FS_String objName;
    while(_usableBlockHeader != 0)
    {
        MemoryBlock *header = _usableBlockHeader;
        _usableBlockHeader = _usableBlockHeader->_nextBlock;

        // й©����
        objName = header->_objName;
        auto iterAmount = objNameRefAmount.find(objName);
        if(iterAmount == objNameRefAmount.end())
            iterAmount = objNameRefAmount.insert(std::make_pair(objName, 0)).first;
        iterAmount->second += 1;

        if(!header->_isInPool)
            ::free(header);
    }
    _usableBlockHeader = 0;

    if(_buf)
        ::free(_buf);

    // �ڴ�й©��ӡ
    for(auto &memleakObj : objNameRefAmount)
        g_Log->memleak("objName[%s], amount[%lld];", memleakObj.first.c_str(), memleakObj.second);
}

size_t IMemoryAlloctor::_GetFreeBlock()
{
    MemoryBlock *temp = _usableBlockHeader;
    size_t cnt = 0;
    while(temp != 0)
    {
        ++cnt;
        temp = temp->_nextBlock;
    }
    return  cnt;
}
FS_NAMESPACE_END

