/**************************************************************
*
*FileName: UniversalNode.h
*Creator : Zhangzhe
*   Date : 2012-11-19
*Comment : CUniversalNode�Ǵ�ͼԪ����CGeometry�̳У�ʵ���˻�����
*           �Ľӿڷ������������Ӧ�����Ժͷ���ʹ������
*           ��XMLTree����չʾ���������Ҳ������(CAttrNode)
*           �ı�(CTextNode)�ͽ������(CEldementNode)
*           �Ļ��࣬�������ߵĹ������ԡ�
/**************************************************************/
#ifndef _UNIVERSALNODE_H
#define _UNIVERSALNODE_H

#include "Afxtempl.h"
#include "Geometry.h"

class CNodeManage;
class CUniversalNode : public CGeometry
{
protected:
	CObList         m_listChild;      //���к��ӽ��ָ��	
	CUniversalNode *m_pParent;        //���׽��ָ��
	CPoint          m_centerPos;      //�ý���λ����Ϣ
    CSize           m_Size;           //�����ռ����Χ
	CString         m_strContent;     //�ý����ʾ���ı�����
    COLORREF        m_bgColor;        //��㱳����ɫ
    COLORREF        m_textColor;      //������ɫ
    int             m_iID;            //���ID
	CPoint          m_ParentPos;      //���ڵ�λ��---�ñ����������л��洢
	CArray<CPoint,CPoint> m_SonsPos;  //���ӽ���λ��---�������������л��洢
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
