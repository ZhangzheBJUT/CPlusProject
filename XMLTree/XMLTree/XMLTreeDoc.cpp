#include "stdafx.h"
#include "XMLTree.h"
#include "XMLTreeDoc.h"
#include "XMLTextView.h"
#include "NodeManage.h"
#include "XMLParse.h"

IMPLEMENT_DYNCREATE(CXMLTreeDoc, CDocument)

BEGIN_MESSAGE_MAP(CXMLTreeDoc, CDocument)
	//{{AFX_MSG_MAP(CXMLTreeDoc)
	ON_COMMAND(ID_UPDATE, OnUpdateContent)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
/************************************************************************/
/*description:  Constructor 
/*  arguments:  void
/*    return:   none                                                   
/************************************************************************/
CXMLTreeDoc::CXMLTreeDoc()
{
	m_pManage = CNodeManage::getInstance();
 	m_pPrase = new CXMLParse();
}
/************************************************************************/
/*description:  Destructor
/*  arguments:  void
/*    return:   none                                                   
/************************************************************************/
CXMLTreeDoc::~CXMLTreeDoc()
{
	delete m_pPrase;
}
/************************************************************************/
/*description:  init the variables
/*  arguments:  void
/*    return:   BOOL  TRUE: need to refresh the screen.
/*                   FALSE: need not to refresh the screen.                                                   
/************************************************************************/
BOOL CXMLTreeDoc::generateNodes()
{
	//������Ϊ��ʱ
	if (m_strContent.GetLength() == 0) 
	{
	    AfxMessageBox("Input None!");
		m_pManage->DestroyTree();
		return TRUE;
	}
   
	//1.�ж������xml��Ϣ�Ƿ����xml�淶
	BOOL isSuccess = m_pPrase->startParse(m_strContent,STRING);
	if(FALSE == isSuccess)
	{
		AfxMessageBox("Input Error!");
		return FALSE;
	}
	
	//2.��յ�ǰ�����д洢�Ľ����Ϣ
	m_pManage->DestroyTree();
	
	//3.��ʼ�������ɽ�㲢�������Ӳ�ι�ϵ
	m_pManage->initTreeFromXML(m_pPrase);

	return TRUE;
}
/************************************************************************/
/*description: called when user clicked the 'update' menu.
/*             update the XMLTreeView and XMLTextView.
/*             
/*  arguments: void
/*               
/*    return:  void                                            
/************************************************************************/
void CXMLTreeDoc::OnUpdateContent() 
{
	POSITION pos = GetFirstViewPosition();
	CXMLTextView *textView = (CXMLTextView*)GetNextView(pos); 
	
	textView->getContent();
	m_strContent = textView->m_strContent;
    
	if (TRUE == generateNodes())
	    this->UpdateAllViews(NULL);
}
/************************************************************************/
/*description: serialize the tree nodes.
/*             
/*  arguments: ar: set by system.
/*               
/*    return:  void                                            
/************************************************************************/
void CXMLTreeDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		//�������ڵ�洢�丸�ӽ���λ��
		m_pManage->storeTree();
		//�洢xml�ı���Ϣ
		ar<<m_strContent;
		//�洢xmlͼ����Ϣ
		m_pManage->m_listNodes.Serialize(ar);
	}
	else
	{
		ar>>m_strContent;	
		m_pManage = CNodeManage::getInstance();
		m_pManage->m_listNodes.Serialize(ar);
		//ͨ���õ��ĸ��ӽ���������ϵ�ؽ�������Ĺ�ϵ
        m_pManage->restoreTree();
	} 
}
BOOL CXMLTreeDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}
#ifdef _DEBUG
void CXMLTreeDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CXMLTreeDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CXMLTreeDoc commands

BOOL CXMLTreeDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{	
	return CDocument::OnSaveDocument(lpszPathName);
}

BOOL CXMLTreeDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;	
	return TRUE;
}
