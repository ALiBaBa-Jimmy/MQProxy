/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��5��9��
**************************************************************************/
#ifndef __DIAM_MESSAGEBLOCK_H__
#define __DIAM_MESSAGEBLOCK_H__

#include <util/diam_ace.h>
#include <parser/diam_avpallocator.h>

/**************************************************************************
�� �� ��: AAAMessageBlock
��������: �̳�ACE��ACE_Message_Block��ʵ�ֶԽ��ܵ���buffer����
��    ��:
�� �� ֵ:
**************************************************************************/
class DiamMsgBlock: public ACE_Message_Block
{
public:
    /**************************************************************************
    �� �� ��:
    ��������:
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    static DiamMsgBlock* Acquire(char *buf, ACE_UINT32 s) {
        return new DiamMsgBlock(buf,s);
    }

    /**************************************************************************
    �� �� ��:
    ��������:
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    static DiamMsgBlock* Acquire(ACE_UINT32 s) {
        return new DiamMsgBlock(s);
    }

    /**************************************************************************
    �� �� ��:
    ��������:
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    static DiamMsgBlock* Acquire(DiamMsgBlock* b) {
        return new DiamMsgBlock(b);
    }

    /**************************************************************************
    �� �� ��:
    ��������:
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    void Release() {
        release();
    }

protected:
    /**************************************************************************
    �� �� ��:
    ��������:
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    DiamMsgBlock(char *buf, ACE_UINT32 s) {
        init(buf, s);
    }
    /**************************************************************************
    �� �� ��:
    ��������:
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    DiamMsgBlock(ACE_UINT32 s) {
        init(s);
    }
    /**************************************************************************
    �� �� ��:
    ��������:
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    DiamMsgBlock(DiamMsgBlock *b) : ACE_Message_Block((const ACE_Message_Block&)*b,0) {}
    /**************************************************************************
    �� �� ��:
    ��������:
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    void* operator new(size_t s)
    {
        return containerMalloc(s);
    }
    /**************************************************************************
    �� �� ��:
    ��������:
    ��    ��:
    �� �� ֵ:
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
