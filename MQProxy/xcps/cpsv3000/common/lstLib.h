/******************************************************************************

                  版权所有 (C), 2001-2014, 深圳信威通信技术有限公司

 ******************************************************************************
  文 件 名   : lstLib.h
  版 本 号   : 初稿
  作    者   : liujun
  生成日期   : 2014年10月30日
  最近修改   :
  功能描述   : 头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年10月30日
    作    者   : liujun
    修改内容   : 创建文件

******************************************************************************/

#ifndef __INClstLibh
#define __INClstLibh

#define LIST_ERROR        -1
#define LIST_SUCCESS      0

#ifdef __cplusplus
extern "C" {
#endif


/* type definitions */

typedef struct node     /* Node of a linked list. */
    {
    struct node *next;      /* Points at the next node in the list */
    struct node *previous;  /* Points at the previous node in the list */
    } NODE;


/* HIDDEN */

typedef struct          /* Header for a linked list. */
    {
    NODE node;          /* Header list node */
    int count;          /* Number of nodes in list */
    } LIST;

/* END_HIDDEN */


/* function declarations */


#if defined(__STDC__) || defined(__cplusplus)

extern void lstLibInit (void);
extern NODE *   lstFirst (LIST *pList);
extern NODE *   lstGet (LIST *pList);
extern NODE *   lstLast (LIST *pList);
extern NODE *   lstNStep (NODE *pNode, int nStep);
extern NODE *   lstNext (NODE *pNode);
extern NODE *   lstNth (LIST *pList, int nodenum);
extern NODE *   lstPrevious (NODE *pNode);
extern int  lstCount (LIST *pList);
extern int  lstFind (LIST *pList, NODE *pNode);
extern void     lstAdd (LIST *pList, NODE *pNode);
extern void     lstConcat (LIST *pDstList, LIST *pAddList);
extern void     lstDelete (LIST *pList, NODE *pNode);
extern void     lstExtract (LIST *pSrcList, NODE *pStartNode, NODE *pEndNode,
                LIST *pDstList);
extern void     lstFree (LIST *pList);
extern void     lstInit (LIST *pList);
extern void     lstInsert (LIST *pList, NODE *pPrev, NODE *pNode);

#else   /* __STDC__ */

extern void lstLibInit ();
extern NODE *   lstFirst ();
extern NODE *   lstGet ();
extern NODE *   lstLast ();
extern NODE *   lstNStep ();
extern NODE *   lstNext ();
extern NODE *   lstNth ();
extern NODE *   lstPrevious ();
extern int  lstCount ();
extern int  lstFind ();
extern void     lstAdd ();
extern void     lstConcat ();
extern void     lstDelete ();
extern void     lstExtract ();
extern void     lstFree ();
extern void     lstInit ();
extern void     lstInsert ();

#endif  /* __STDC__ */

#ifdef __cplusplus
}
#endif

#endif /* __INClstLibh */
