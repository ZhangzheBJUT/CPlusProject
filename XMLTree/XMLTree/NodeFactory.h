/**************************************************************
*
*FileName: NodeFactory.h
*Creator : Zhangzhe
*   Date : 2012-12-14
*Comment : NodeFactory������CElementNode��CAttrNode
*           ��CTextNode�Ĺ����࣬������ֽ��Ĵ���
/**************************************************************/

#ifndef _NODEFACTORY_H
#define _NODEFACTORY_H

class CUniversalNode;
class CElementNode;
class CAttrNode;
class CTextNode;

class CNodeFactory
{
public:
	CNodeFactory();
    CUniversalNode* getElementNode(CString content,int x=0,int y=0);
	CUniversalNode* getAttrNode(CString content,int x=0,int y=0);
	CUniversalNode* getTextNode(CString content,int x=0,int y=0);
	virtual ~CNodeFactory();
};
#endif//endif
