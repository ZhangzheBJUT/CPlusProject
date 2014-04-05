/**************************************************************
*
*FileName: TextNode.h
*Creator : Zhangzhe
*   Date : 2012-11-20
*Comment : 该类是从CUniversalNode派生的，用于显示结点的文本信息。
*
/**************************************************************/
#ifndef  _TEXTNODE_H
#define _TEXTNODE_H

#include "UniversalNode.h"
class CTextNode : public CUniversalNode
{
protected:
	CTextNode();
	DECLARE_SERIAL(CTextNode)
public:
	CTextNode(CString content,int x=0,int y=0);
	virtual void Draw(CDC *pdc=NULL);
	virtual void Serialize(CArchive& ar);
	virtual ~CTextNode();
};
#endif //endif
