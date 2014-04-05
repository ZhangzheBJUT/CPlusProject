#include "stdafx.h"
#include "UniversalNode.h"
#include "constants.h"
#include "NodeManage.h"
/************************************************************************/
/*description:  Constructor
/*  arguments:  content: the text to be display on the node.  
/*                    x: the x coordinates   
/*                    y: the y coordinates
/*    return:   none                                                   
/************************************************************************/
CUniversalNode::CUniversalNode(CString content,int x,int y)
{
	m_centerPos.x = x;
	m_centerPos.y = y;
    m_strContent  = content;
	
	m_Size.cy = DEFAULT_HEIGHT;     
	m_Size.cx = DEFAULT_WIDTH;
	
	m_bgColor   = YELLOW;
    m_textColor = BLACK;
    
    m_iID       = 260;
	
   	m_pParent   = NULL;
}
CUniversalNode::CUniversalNode()
{
	m_pParent = NULL;
}
CUniversalNode::~CUniversalNode()
{
}
/************************************************************************/
/*description:  add a son node to current node.
/*  arguments:  son :the son node to be added.
/*    return:   none                                                   
/************************************************************************/
void CUniversalNode::addSon(CUniversalNode *son)
{
	m_listChild.AddTail(son);
}
/************************************************************************/
/*description:  set a parent node for the current node.
/*  arguments:  parent :the parent node to be set.
/*    return:   none                                                   
/************************************************************************/
void CUniversalNode::setParent(CUniversalNode *parent)
{
	m_pParent = parent;
}
/************************************************************************/
/*description:  before serialize nodes into file,store the
/*              node's parent and sons' coordinates into the array. 
/*  arguments:  void
/*    return:   void                                                             
/************************************************************************/
void CUniversalNode::storePosInfo()
{
	//如果是根结点设置其父节点坐标为(-1,-1)
	if (m_pParent == NULL)
		m_ParentPos = CPoint(-1,-1);
	else 
		m_ParentPos = m_pParent->m_centerPos;
	
    //存储其子节点坐标
	POSITION ps = m_listChild.GetHeadPosition();
	while (ps)
	{
		CUniversalNode *node =  (CUniversalNode*)m_listChild.GetNext(ps);
		CPoint pt(node->m_centerPos);
		m_SonsPos.Add(pt);
	}
}
/************************************************************************/
/*description:  after serialize nodes from file,restore the 
/*              nodes' relationship based the coordinates.     
/*  arguments:  void
/*    return:   void                                                             
/************************************************************************/
void CUniversalNode::buildRelation(CNodeManage *nodeManage)
{
	CObList &list = nodeManage->m_listNodes;
	POSITION ps;
	
	if (m_ParentPos.x == -1 && m_ParentPos.y == -1) //说明是根结点
	{
		m_pParent = NULL;
		nodeManage->m_root = this;
	}
    else
	{
		ps = list.GetHeadPosition();
		while (ps)
		{
			CUniversalNode* node = (CUniversalNode*)list.GetNext(ps);
			if (m_ParentPos.x == node->m_centerPos.x &&  //如果是父节点的话
				m_ParentPos.y == node->m_centerPos.y)
			{
				m_pParent = node;
				break;
			}
		}//else
	}//if
    
	int count = m_SonsPos.GetSize();
	for (int i=0; i<m_SonsPos.GetSize();++i)
	{
		ps = list.GetHeadPosition();
        while (ps)
		{
			CUniversalNode* node = (CUniversalNode*)list.GetNext(ps);
			if (m_SonsPos.GetAt(i).x == node->m_centerPos.x &&  //说明是其子
				m_SonsPos.GetAt(i).y == node->m_centerPos.y)
			{
				m_listChild.AddTail(node);
			}
		}//while
	}//for
}
void CUniversalNode::offset(int cx,int cy)
{
	m_centerPos.x += cx;
	m_centerPos.y += cy;
}
BOOL CUniversalNode::clickTest(CPoint point)
{
	int left   = m_centerPos.x-m_Size.cx/2;
	int top    = m_centerPos.y-m_Size.cy/2;
	int right  = m_centerPos.x+m_Size.cx/2; 
	int bottom =  m_centerPos.y+m_Size.cy/2;
	
	if (left<=point.x && point.x<=right 
		&& top<=point.y && bottom>=point.y)
	{
		return TRUE;
	}
	
	return FALSE;
}
