/**************************************************************
*
*FileName: NodeManage.h
*Creator : Zhangzhe
*   Date : 2012-11-24
*Comment : CNodeManage������CElementNode��CAttrNode
*           ��CTextNode�Ĺ����࣬����ά������Ԫ�ص���
*           �ӡ�ɾ����λ�õ�ȷ���ȹ�����
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
	
	void storeTree();     //�������л�
	void restoreTree();   //���ڷ����л�
private:
	                       //���ڲ�λ�������״
	void levelTraversal();
	void depthTraversal(CXMLParse* parse,IXMLDOMNodePtr current,
		                int depth,CUniversalNode* parent=NULL);
	
	int setLocation_X(CView* view=NULL,int num   = 0);
	int setLocation_Y(CView* view=NULL,int depth = 0);
	void treeNodesLink(CUniversalNode* node=NULL,CDC* pDC = NULL);
public:
	static CNodeManage *m_nodeManage;   //Singleton: store the pointer
    CUniversalNode     *m_selectedNode;
	CUniversalNode     *m_root;         //�������
	CObList             m_listNodes;    //���ָ������
};
#endif