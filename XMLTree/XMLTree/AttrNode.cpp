#include "stdafx.h"
#include "UniversalNode.h"
#include "constants.h"
#include "AttrNode.h"

IMPLEMENT_SERIAL(CAttrNode,CButton,1)
/************************************************************************/
/*description:  Constructor
/*  arguments:  content: the text to be display on the node.  
/*                    x: the x coordinates   
/*                    y: the y coordinates
/*    return:   none                                                   
/************************************************************************/
CAttrNode::CAttrNode(CString content,int x,int y):
                     CUniversalNode(content,x,y)
{
	m_Size.cx = ATTRNODE_WIDTH ;  //Width
	m_Size.cy = ATTRNODE_HEIGHT;  //Height
}
CAttrNode::CAttrNode()
{
	m_pParent = NULL;
}
/************************************************************************/
/*description:  draw the Attribute Node style  button.
/*  arguments:  DC: the device context used to draw.  
/*            rect: the rect area to draw on.
/*           color: the filled color.   
/*    return:   void                                                   
/************************************************************************/
void CAttrNode::Draw(CDC *pdc)
{
	CBrush b;
	b.CreateSolidBrush(m_bgColor);
	pdc->SetBkMode(TRANSPARENT);
	CBrush *old = pdc->SelectObject(&b);

	int left   = m_centerPos.x-m_Size.cx/2+1;
	int top    = m_centerPos.y-m_Size.cy/2+1;
	int right  = m_centerPos.x+m_Size.cx/2+1; 
	int bottom =  m_centerPos.y+m_Size.cy/2+1;
	
	CRect rect(left,top,right,bottom);
	pdc->Ellipse(rect);
	pdc->TextOut(left+10,(top+bottom)/2-5,m_strContent);
	
	pdc->SelectObject(old);
}
void CAttrNode::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_strContent;      //��ʾ����
		ar << m_Size;            //����С
		ar << m_centerPos;       //���λ��
        ar << m_bgColor;         //��ɫ����
		ar << m_textColor;
		ar << m_iID;             //���ID
		ar << m_ParentPos;       //���ڵ�λ��
		m_SonsPos.Serialize(ar); //�ӽڵ�λ��
		
	}
	else
	{
		ar >> m_strContent;      //��ʾ����
		ar >> m_Size;            //����С
		ar >> m_centerPos;       //���λ��
        ar >> m_bgColor;         //��ɫ����
		ar >> m_textColor;
		ar >> m_iID;             //���ID
		ar >> m_ParentPos;       //���ڵ�λ��
		m_SonsPos.Serialize(ar); //�ӽڵ�λ��
	}
}
CAttrNode::~CAttrNode()
{
}
