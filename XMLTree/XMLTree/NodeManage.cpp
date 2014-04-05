#include "StdAfx.h"
#include "UniversalNode.h"
#include "constants.h"
#include "NodeManage.h"
#include "NodeFactory.h"

CNodeManage* CNodeManage::m_nodeManage = NULL;
BOOL  isParsed = FALSE;
CView *m_view    = NULL;
/************************************************************************/
/*
/*description:  get a CNodeManage instance if it do not exist.
/*  arguments:           view:  pointer to the nodes' parent. 
/*                      parse:  pointer to the parse class.
/*    return:   CNodeManage* :  get a CNodeManange instance.
/*                                                                                
/************************************************************************/
CNodeManage* CNodeManage::getInstance()
{
	if (NULL == m_nodeManage)
		m_nodeManage = new CNodeManage();
	
	return m_nodeManage;
}
/************************************************************************/
/*description:  constructor
/*  arguments:           view:  pointer to the nodes' parent. 
/*                      parse:  pointer to the parse class.
/*    return:   none                                                   
/************************************************************************/
CNodeManage::CNodeManage()
{
	m_view         = NULL;
	m_selectedNode = NULL;
	m_root         = NULL;
}
/************************************************************************/
/*description:  set the root node.
/*  arguments:  root:  pointer to the tree root.                       
/*    return:   void                                                   
/************************************************************************/
void CNodeManage::setTreeRoot(CUniversalNode* root)
{
	if (root)
		m_root = root;
}
/************************************************************************/
/*description:  get the root node.
/*  arguments:  void                     
/*    return:   CUniversalNode*: root pointer                                                   
/************************************************************************/
CUniversalNode* CNodeManage::getTreeRoot()
{
	return m_root;
}	
/************************************************************************/
/*description:  add a node to the list.
/*  arguments:  parent: the parent of the son which will be added
/*                      to the list. 
/*                 son: the node will be added to the list.                    
/*    return:   void                                                   
/************************************************************************/
void CNodeManage::addTreeNodes(CUniversalNode* parent,CUniversalNode* son)
{
	if (parent != NULL)
		parent->addSon(son);
	else
	    m_root = son;
	
	son->setParent(parent);
	m_listNodes.AddTail(son);
}
/************************************************************************/
/*description:  remove all the nodes from the list.
/*  arguments:  void                    
/*    return:   void                                                   
/************************************************************************/
void CNodeManage::DestroyTree()
{
	if (m_listNodes.IsEmpty() == 0)
	{
		POSITION ps = m_listNodes.GetHeadPosition();
        while (ps !=0 )
        {
			CUniversalNode *temp = (CUniversalNode *)m_listNodes.GetNext(ps);
		    delete temp;
			temp = NULL;
		}
		m_listNodes.RemoveAll();
		m_root = NULL;
	}
}
/************************************************************************/
/*description:  Level-order Traversal the tree to set the node position
/*              int the parent view.
/*  arguments:  void                    
/*    return:   void                                                   
/************************************************************************/
void CNodeManage::levelTraversal()
{
    int depth = 1;
    int   num = 0;
    CObList queue;

	queue.AddTail(m_root);

	while (FALSE == queue.IsEmpty())
	{
		CUniversalNode* node=(CUniversalNode*)queue.RemoveHead();
		node->m_iID = 0;      
		if (depth == node->m_centerPos.y)
		{
			node->m_centerPos.x = num++;
		}
		else
		{
			++depth;num = 1; 
			node->m_centerPos.x = num++;
		}

        POSITION psChild = node->m_listChild.GetHeadPosition();
		while (psChild)
		{
			CUniversalNode *son = (CUniversalNode*)node->m_listChild.GetNext(psChild);
			if (son->m_iID != 0)
				queue.AddTail(son);
			
		}
	}
}
/************************************************************************/
/*description:  to set the x-coordinate in the parent view.
/*  arguments:  num : the sequence number in a level.                   
/*    return:   int : the node x-coordinate.                                               
/************************************************************************/
int CNodeManage::setLocation_X(CView *view,int num)
{
	CRect rect;
	view->GetClientRect(&rect); 

	int max_X= rect.Width(); 
	//x轴方向最多可以存放的结点数
	int max_NUM = max_X/(DEFAULT_WIDTH+10);
	//根结点横坐标
	if (num == 0)
		return max_X/2;
	
    if (num>max_NUM)
		num = max_NUM;
    
    return  (DEFAULT_WIDTH+10)*num;
}
/************************************************************************/
/*description:  to set the Y-coordinate in the parent view.
/*  arguments:  depth : the node's depth in the tree.                   
/*    return:     int : the node's Y-coordinate in the parente view.                                              
/************************************************************************/
int CNodeManage::setLocation_Y(CView* view,int depth)
{
	CRect rect;
	view->GetClientRect(&rect); 
	
	int max_Y = rect.Height();
	int max_NUM = max_Y/(DEFAULT_HEIGHT+10);
	//根结点纵坐标
	if (depth == 1) 
		return DEFAULT_HEIGHT/2;
	
	if (depth>max_NUM)
		depth = max_NUM;
	
	return (DEFAULT_HEIGHT+10)*depth;
}
/************************************************************************/
/*description:   set the node position  in the parent view.
/*  arguments:  current : the current node's address in the DOMTree.                   
/*                depth : the current node's depth in the DOMTree.  
/*                parent: the current node's parent. 
/*     return:   void                                          
/************************************************************************/
void CNodeManage::initTreeFromXML(CXMLParse *parse)
{
	depthTraversal(parse,parse->getDocumentRoot(),1,NULL);
	levelTraversal();
	isParsed = TRUE;
}
/************************************************************************/
/*description:   depth traversal the tree to set the node position in the 
/*               parent view.
/*  arguments:  current : the current node's address in the DOMTree.                   
/*                depth : the current node's depth in the DOMTree.  
/*                parent: the current node's parent. 
/*     return:   void                                          
/************************************************************************/
void CNodeManage::depthTraversal(CXMLParse* parse,IXMLDOMNodePtr current,
								  int depth,CUniversalNode* parent)
{
	
	CUniversalNode *son     = NULL;
	IXMLDOMNodePtr nodeItem = NULL;  
	CNodeFactory nodeFactory;        //用于创建树结点的工厂类
	int depthNum = depth;            //记录当前树的深度    

    /************************************************************************/
	//1. 获得当前结点的名字
    /************************************************************************/
	CString ElementName;
    parse->getElementName(current,ElementName);
	son = nodeFactory.getElementNode(ElementName,0,depthNum);
	m_nodeManage->addTreeNodes(parent,son);
	
    parent = son;                    //son成为下面结点的父节点
    ++depthNum;                      //树的深度加1
	
    /************************************************************************/
	//2. 取得当前结点的属性
    /************************************************************************/
	if (parse->hasAttribute(current)) //如果属性存在的话
	{
		CMapStringToString strMap;
		parse->getElementAttr(current,strMap);
		POSITION ps = strMap.GetStartPosition();
		while (ps)
		{
			CString key;
			CString Value;
			strMap.GetNextAssoc(ps,key,Value);
			//进行字符串拼接格式化字符串
			CString temp = "=";
			key+=temp+"'"+Value+"'";
            CUniversalNode *attrNode  = nodeFactory.getAttrNode(key,0,depthNum);
			m_nodeManage->addTreeNodes(parent,attrNode);
		}//while
	}//if
    /************************************************************************/
	//3. 取得当前结点的文本
    /************************************************************************/
    IXMLDOMNodeListPtr  children = NULL;
	int count = parse->getChildren(current,children);
	if (count==0)
		return;
	//如果存在孩子的话
	for (int i=0;i<count;++i)
	{
		children->get_item(i,&nodeItem);
		 //如果当前的孩子结点是一个文本的话
        if (parse->isNodeText(nodeItem))
        {
			CString text;
			parse->getElementText(nodeItem,text);
			CUniversalNode *textNode = nodeFactory.getTextNode(text,0,depthNum);
		    m_nodeManage->addTreeNodes(parent,textNode);
        }
		else
		{
			depthTraversal(parse,nodeItem,depthNum,parent);
		}
	}
}
/************************************************************************/
/*description:  draw links between the nodes.
/*  arguments:  pDC : the parent's device context.
/*              node: the node drawn from.
/*    return:   void                                                             
/************************************************************************/
void CNodeManage::treeNodesLink(CUniversalNode* node,CDC* pDC)
{
	//首先向其父节点画线
	if (node->m_pParent!=NULL)
	{
		pDC->MoveTo(node->m_centerPos.x,node->m_centerPos.y-node->m_Size.cy/2);
		pDC->LineTo(node->m_pParent->m_centerPos.x,
			node->m_pParent->m_centerPos.y+node->m_pParent->m_Size.cy/2);
	}
	//依次递归的为子孙结点画线
	for (POSITION pos = node->m_listChild.GetHeadPosition();pos!=NULL;)
	{
		CUniversalNode *child = (CUniversalNode*)node->m_listChild.GetNext(pos);
		pDC->MoveTo(node->m_centerPos.x,node->m_centerPos.y+node->m_Size.cy/2);
		pDC->LineTo(child->m_centerPos.x,child->m_centerPos.y-child->m_Size.cy/2);
		treeNodesLink(child,pDC);
	}	
}
/************************************************************************/
/*description:  draw links between the nodes.
/*  arguments:  pDC : the parent's device context.
/*              node: the node drawn from.
/*    return:   void                                                             
/************************************************************************/
void CNodeManage::storeTree()
{
	POSITION ps = m_listNodes.GetHeadPosition();
	while (ps)
	{
		CUniversalNode* node = (CUniversalNode*)m_listNodes.GetNext(ps);
		node->storePosInfo();
	}
}
/************************************************************************/
/*description:  draw links between the nodes.
/*  arguments:  pDC : the parent's device context.
/*              node: the node drawn from.
/*    return:   void                                                             
/************************************************************************/
void CNodeManage::restoreTree()
{
	POSITION ps = m_listNodes.GetHeadPosition();
	while (ps)
	{
		CUniversalNode *node = (CUniversalNode*)m_listNodes.GetNext(ps);
		int count = node->m_SonsPos.GetSize();
		node->buildRelation(this);
	}
}
BOOL CNodeManage::clickTest(CPoint point)
{
	POSITION ps = m_listNodes.GetHeadPosition();
	while (ps)
	{
		CUniversalNode *node = (CUniversalNode*)m_listNodes.GetNext(ps);
		BOOL test = node->clickTest(point);
		if (test)
		{
			m_selectedNode = node;
			return TRUE;
		}
	}
    return FALSE;
}
/************************************************************************/
/*description:   show the tree in the parent view
/*  arguments:   pdc: device context set by the system.
/*     return:   void                                          
/************************************************************************/
void CNodeManage::Draw(CDC *pdc) 
{
	if (m_root)
		treeNodesLink(m_root,pdc);
}
void CNodeManage::showTree(CView *view,CDC *pdc)
{
	m_view = view;
	POSITION ps = m_listNodes.GetHeadPosition();
	while (ps)
	{
	  	CUniversalNode *node = (CUniversalNode*)m_listNodes.GetNext(ps);
		if (isParsed)
		{
			node->m_centerPos.x = setLocation_X(view,node->m_centerPos.x);
			node->m_centerPos.y = setLocation_Y(view,node->m_centerPos.y);
		}
        node->Draw(pdc);
	}
	isParsed = FALSE;
	Draw(pdc);
}
void CNodeManage::offset(int cx,int cy)
{
	//进行边界检测
    CRect rect;
	m_view->GetClientRect(&rect); 

	int right  = rect.Width(); 
	int bottom = rect.Height();
	int left   = 10;
	int top    = 10;
	
	CPoint center;
	int x=0,y=0;

	if (m_selectedNode != NULL)
	{
		center = m_selectedNode->m_centerPos;
		x = center.x + cx;
		y = center.y + cy;

		if (x>=left && x<=right &&
			y>=top  && y<=bottom)
		  m_selectedNode->offset(cx,cy);
	}
}
/************************************************************************/
/*description:  find the node between the start node and end node.
/*  arguments:  aStart : the name of start node.
/*                 aEnd: the name of end node.
/*    return:   TRUE:  find succeefully.
/*             FALSE:  not find.                                                             
/************************************************************************/
BOOL CNodeManage::findNodes(CString aStart,CString aEnd)
{
	CObList startNodes;
	CObList   endNodes;
    BOOL   isFind    = FALSE;
	BOOL   isSuccess = FALSE;
	POSITION ps = m_listNodes.GetHeadPosition();
	//1.找开始结点
	while (ps)
	{
		CUniversalNode* node = (CUniversalNode*)m_listNodes.GetNext(ps);
		if (node->m_strContent == aStart)
		{
			node->m_bgColor = RED;
			startNodes.AddTail(node);
		}
	}
	//2.找结束结点
	ps = m_listNodes.GetHeadPosition();
	while (ps)
	{
		CUniversalNode *node = (CUniversalNode*)m_listNodes.GetNext(ps);
		if (node->m_strContent == aEnd)
		{			
			node->m_bgColor = RED;
			endNodes.AddTail(node);
		}
	}
	//3.从结束结点回溯找1中的父节点
	if (startNodes.IsEmpty() || endNodes.IsEmpty())
		return FALSE;

	for (ps=endNodes.GetHeadPosition();ps!=NULL;)
	{
		CUniversalNode *node = (CUniversalNode*)endNodes.GetNext(ps);
        CUniversalNode *nodeParent = node->m_pParent;
		while (NULL != nodeParent)//如果结束结点存在父节点的话
		{
			POSITION parentPos;
			CUniversalNode *Parent = NULL;
			isFind = FALSE;
			//遍历每一个开始结点，看其是否为结束结点的直接父节点
			for (parentPos=startNodes.GetHeadPosition();parentPos!=NULL;)
			{
			    Parent = (CUniversalNode*)startNodes.GetNext(parentPos);
				if (nodeParent == Parent)
				{
					isFind = TRUE;
					break;
				}
			}//for
            //如果不是其直接父节点的话
			if (isFind == FALSE)
				nodeParent = nodeParent->m_pParent;
			else//说明其祖先路径上存在开始结点
			{
				isSuccess = TRUE;
				CUniversalNode* tempNode = node->m_pParent;
				while (tempNode != Parent)
				{
					tempNode->m_bgColor = RED;
					tempNode = tempNode->m_pParent;
				}
				break;
			}//else
		}//while
	}//for
	return isSuccess;
}

/************************************************************************/
/*description:  clear the find path.
/*  arguments:  void
/*    return:   void                                                             
/************************************************************************/
void CNodeManage::clearFindPath()
{
	POSITION ps = m_listNodes.GetHeadPosition();
	
	while (ps)
	{
		CUniversalNode *node = (CUniversalNode*)m_listNodes.GetNext(ps);
		node->m_bgColor = YELLOW;
	}
}