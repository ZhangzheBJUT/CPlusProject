/**************************************************************
*
*FileName: AttrNode.h
*Creator : Zhangzhe
*   Date : 2012-11-20
*Comment : 该类是从CUniversalNode派生的用于显示结点属性名称。
*
/**************************************************************/
#ifndef _ATTRNODE_H
#define _ATTRNODE_H

#include "UniversalNode.h"

class CUniversalNode;
class CAttrNode : public CUniversalNode
{
protected:
	CAttrNode();
	DECLARE_SERIAL(CAttrNode)
public:
	CAttrNode(CString content,int x=0,int y=0);
	virtual void Draw(CDC *pdc=NULL);
	virtual void Serialize(CArchive& ar);
	virtual ~CAttrNode();
};
#endif//endif
