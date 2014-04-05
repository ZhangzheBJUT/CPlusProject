/**************************************************************
*
*FileName: NodeManage.h
*Creator : Zhangzhe
*   Date : 2012-11-24
*Comment : CNodeManage是用于CElementNode、CAttrNode
*           和CTextNode的管理类，负责维护三种元素的添
*           加、删除、位置的确定等工作。
*
/**************************************************************/
#ifndef _NODEMANAGE_H
#define _NODEMANAGE_H

#include "XMLParse.h"
#include "Geometry.h"
class CUniversalNode;

class CNodeManage :public CGeometry
{
protected:
	CNodeManage();
public:
	static CNodeManage* getInstance();
	
	void setTreeRoot(CUniversalNode* root = NULL);
	CUniversalNode* getTreeRoot();

	void initTreeFromXML(CXMLParse *parse = NULL);
	void addTreeNodes(CUniversalNode* parent=NULL,CUniversalNode* son=NULL);
	void showTree(CView *view=NULL,CDC *pdc=NULL);
	void DestroyTree();

	virtual BOOL clickTest(CPoint point);
	virtual void Draw(CDC *pdc=NULL);           
    virtual void offset(int cx=0,int cy=0);

    BOOL findNodes(CString aStart,CString aEnd);
    void clearFindPath();
	
	void storeTree();     //用于序列化
	void restoreTree();   //用于反序列化
private:
	                       //用于层次化树的形状
	void levelTraversal();
	void depthTraversal(CXMLParse* parse,IXMLDOMNodePtr current,
		                int depth,CUniversalNode* parent=NULL);
	
	int setLocation_X(CView* view=NULL,int num   = 0);
	int setLocation_Y(CView* view=NULL,int depth = 0);
	void treeNodesLink(CUniversalNode* node=NULL,CDC* pDC = NULL);
public:
	static CNodeManage *m_nodeManage;   //Singleton: store the pointer
    CUniversalNode     *m_selectedNode;
	CUniversalNode     *m_root;         //树根结点
	CObList             m_listNodes;    //树种各个结点
};
#endif