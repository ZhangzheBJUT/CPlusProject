/**************************************************************
*
*FileName: ElementNode.h
*Creator : Zhangzhe
*   Date : 2012-11-20
*Comment : 该类是从CUniversalNode派生的用于显示结点名称。
*
/**************************************************************/
#ifndef _ELEMENTNODE_H
#define _ELEMENTNODE_H

#include "UniversalNode.h"
class CElementNode : public CUniversalNode  
{
protected:
	CElementNode();
	DECLARE_SERIAL(CElementNode)
public:
	CElementNode(CString content,int x=0,int y=0);
	virtual void Draw(CDC *pdc=NULL);
	virtual void Serialize(CArchive& ar);
	virtual ~CElementNode();
};
#endif//endif