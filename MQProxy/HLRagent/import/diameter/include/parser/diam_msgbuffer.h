/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年5月9日
**************************************************************************/
#ifndef __DIAM_MESSAGEBLOCK_H__
#define __DIAM_MESSAGEBLOCK_H__

#include <util/diam_ace.h>
#include <parser/diam_avpallocator.h>

/**************************************************************************
函 数 名: AAAMessageBlock
函数功能: 继承ACE的ACE_Message_Block，实现对接受到的buffer操作
参    数:
返 回 值:
**************************************************************************/
class DiamMsgBlock: public ACE_Message_Block
{
public:
    /**************************************************************************
    函 数 名:
    函数功能:
    参    数:
    返 回 值:
    **************************************************************************/
    static DiamMsgBlock* Acquire(char *buf, ACE_UINT32 s) {
        return new DiamMsgBlock(buf,s);
    }

    /**************************************************************************
    函 数 名:
    函数功能:
    参    数:
    返 回 值:
    **************************************************************************/
    static DiamMsgBlock* Acquire(ACE_UINT32 s) {
        return new DiamMsgBlock(s);
    }

    /**************************************************************************
    函 数 名:
    函数功能:
    参    数:
    返 回 值:
    **************************************************************************/
    static DiamMsgBlock* Acquire(DiamMsgBlock* b) {
        return new DiamMsgBlock(b);
    }

    /**************************************************************************
    函 数 名:
    函数功能:
    参    数:
    返 回 值:
    **************************************************************************/
    void Release() {
        release();
    }

protected:
    /**************************************************************************
    函 数 名:
    函数功能:
    参    数:
    返 回 值:
    **************************************************************************/
    DiamMsgBlock(char *buf, ACE_UINT32 s) {
        init(buf, s);
    }
    /**************************************************************************
    函 数 名:
    函数功能:
    参    数:
    返 回 值:
    **************************************************************************/
    DiamMsgBlock(ACE_UINT32 s) {
        init(s);
    }
    /**************************************************************************
    函 数 名:
    函数功能:
    参    数:
    返 回 值:
    **************************************************************************/
    DiamMsgBlock(DiamMsgBlock *b) : ACE_Message_Block((const ACE_Message_Block&)*b,0) {}
    /**************************************************************************
    函 数 名:
    函数功能:
    参    数:
    返 回 值:
    **************************************************************************/
    void* operator new(size_t s)
    {
        return containerMalloc(s);
    }
    /**************************************************************************
    函 数 名:
    函数功能:
    参    数:
    返 回 值:
    **************************************************************************/
    void operator delete(void *p)
    {
        containerFree(p);
    }

private:

    ~DiamMsgBlock() {}
};

class CDiamMessageBlockScope
{
public:
    CDiamMessageBlockScope(ACE_Message_Block *&mb) : mb_(mb) { }

    ~CDiamMessageBlockScope(void) {
        mb_->release();
    }

protected:
    ACE_Message_Block *&mb_;
};

class CDiamMessageBlockManager
{
public:
    /*!
    * Definitions for default block count
    */
    enum
    {
        DIAM_MIN_MESSAGE_COUNT = 512
    };

public:
    /*!
    * constructor
    *
    * \param n_blocks Number of blocks to manage
    */
    CDiamMessageBlockManager(int n_blocks = DIAM_MIN_MESSAGE_COUNT);

    /*!
    * destructor
    */
    ~CDiamMessageBlockManager();

    /*!
    * Access function to message singleton
    */
    static CDiamMessageBlockManager *instance()
    {
        return &CDiamMessageBlockManager::allocator_;
    }

    /*!
    * Allocates an un-used message buffer
    */
    DiamMsgBlock *malloc();

    /*!
    * Returns a message buffer to the free list
    */
    void free(DiamMsgBlock *buffer);

private:
    static CDiamMessageBlockManager allocator_; /*<< Singleton instance of the allocator */

    DiamMsgBlock **pool_; /*<< Message pool */

    int num_blocks_; /*<< number of message buffers in the pool */
};


#endif /*__DIAM_MESSAGEBLOCK_H__*/
