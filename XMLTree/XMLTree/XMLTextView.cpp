#include "stdafx.h"
#include "XMLTree.h"
#include "XMLTextView.h"
#include "XMLTreeDoc.h"

IMPLEMENT_DYNCREATE(CXMLTextView, CEditView)

CXMLTextView::CXMLTextView()
{
}
CXMLTextView::~CXMLTextView()
{
}
BEGIN_MESSAGE_MAP(CXMLTextView, CEditView)
	//{{AFX_MSG_MAP(CXMLTextView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
/************************************************************************/
/*description: get the user input from the edit view.
/*             
/*  arguments: void
/*               
/*    return:  void                                          
/************************************************************************/
void CXMLTextView::getContent()
{
	GetEditCtrl().GetWindowText(m_strContent);  
}
/************************************************************************/
/*description: show and update the edit view content.
/*             
/*  arguments: set by the system.
/*               
/*    return:  void                                          
/************************************************************************/
void CXMLTextView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CXMLTreeDoc *pDoc  = GetDocument();
	//���ĵ����еõ�Ҫ��ʾ���ַ���
    m_strContent = pDoc->m_strContent;
	//��ʾ�ַ���
    GetEditCtrl().SetWindowText(m_strContent);
}
void CXMLTextView::OnDraw(CDC* pDC)
{
	CXMLTreeDoc* pDoc = GetDocument();
}
void CXMLTextView::AssertValid() const
{
	CEditView::AssertValid();
}
void CXMLTextView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
CXMLTreeDoc* CXMLTextView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CXMLTreeDoc)));
	return (CXMLTreeDoc*)m_pDocument;
}
void CXMLTextView::OnInitialUpdate() 
{
	CEditView::OnInitialUpdate();
}

