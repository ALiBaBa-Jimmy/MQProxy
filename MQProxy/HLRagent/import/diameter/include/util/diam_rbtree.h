/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年6月1日
**************************************************************************/
#ifndef __OD_UTL_RBTREE_H__
#define __OD_UTL_RBTREE_H__

typedef enum {
    RBTREE_CLR_BLACK,
    RBTREE_CLR_RED
} util_RbTreeNodeColor;

class util_RbTreeData
{
public:
    // constructor
    util_RbTreeData() {
        payload = this;
    };
    virtual ~util_RbTreeData() {};

    // exposed functions
    void *payload;

    // methods
    virtual int operator < (util_RbTreeData &cmp) = 0;
    virtual int operator == (util_RbTreeData &cmp) = 0;
    virtual void clear(void *userData = 0) = 0;

    // debugging only
    virtual void Dump(void *userData) {};
};

class util_RbTreeNode {
public:
    // constructor
    util_RbTreeNode();

    // exposed members
    util_RbTreeNode *left;
    util_RbTreeNode *right;
    util_RbTreeNode *parent;
    util_RbTreeNodeColor color;
    util_RbTreeData *data;

    // methods
    util_RbTreeNode &operator=(util_RbTreeNode &source);
};

class util_RbTreeTree {
public:
    // constructor
    util_RbTreeTree() {
        root = &nil;
    };

    // methods
    util_RbTreeData *Insert(util_RbTreeData *data);
    util_RbTreeData *Find(util_RbTreeData *data);
    util_RbTreeData *Remove(util_RbTreeData *data);
    void Clear(void *userData = 0);

    // debugging only
    void Dump(util_RbTreeNode *x = 0, void *userData = 0);

private:
    util_RbTreeNode *root;
    util_RbTreeNode nil;

    void FixupInsert(util_RbTreeNode *x);
    void FixupRemove(util_RbTreeNode *x);
    void RotateLeft(util_RbTreeNode *x);
    void RotateRight(util_RbTreeNode *x);
};

#endif /* __OD_UTL_RBTREE_H__ */
