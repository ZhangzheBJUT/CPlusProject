/**************************************************************
*
*FileName: UniversalNode.h
*Creator : Zhangzhe
*   Date : 2012-11-19
*Comment : CUniversalNode是从图元基类CGeometry继承，实现了基类中
*           的接口方法，并添加相应的属性和方法使其适用
*           于XMLTree结点的展示。另外该类也是属性(CAttrNode)
*           文本(CTextNode)和结点名称(CEldementNode)
*           的基类，保存三者的公共属性。
/**************************************************************/
#ifndef _UNIVERSALNODE_H
#define _UNIVERSALNODE_H

#include "Afxtempl.h"
#include "Geometry.h"

class CNodeManage;
class CUniversalNode : public CGeometry
{
protected:
	CObList         m_listChild;      //所有孩子结点指针	
	CUniversalNode *m_pParent;        //父亲结点指针
	CPoint          m_centerPos;      //该结点的位置信息
    CSize           m_Size;           //结点所占区域范围
	CString         m_strContent;     //该结点显示的文本内容
    COLORREF        m_bgColor;        //结点背景颜色
    COLORREF        m_textColor;      //字体颜色
    int             m_iID;            //结点ID
	CPoint          m_ParentPos;      //父节点位置---该变量用于序列化存储
	CArray<CPoint,CPoint> m_SonsPos;  //孩子结点的位置---该数组用于序列化存储
	CUniversalNode();
public:
	CUniversalNode(CString content,int x=0,int y=0);
	void addSon(CUniversalNode *son = NULL);
	void setParent(CUniversalNode *parent = NULL);
	BOOL clickTest(CPoint point);
	void offset(int cx=0,int cy=0);
	void storePosInfo();
	void buildRelation(CNodeManage *nodeManage = NULL);
	~CUniversalNode();
	friend CNodeManage;
};
#endif
