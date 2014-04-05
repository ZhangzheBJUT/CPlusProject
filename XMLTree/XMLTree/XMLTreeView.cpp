#include "stdafx.h"
#include "XMLTree.h"
#include "XMLTreeDoc.h"
#include "XMLTreeView.h"
#include "UniversalNode.h"
#include "NodeManage.h"
#include "FindDlg.h"

IMPLEMENT_DYNCREATE(CXMLTreeView, CScrollView)

BEGIN_MESSAGE_MAP(CXMLTreeView, CScrollView)
	//{{AFX_MSG_MAP(CXMLTreeView)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_COMMAND(IDC_FIND, find)
	ON_COMMAND(ID_MEUN_CLEAR_FIND_PATH, clearFindPath)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
END_MESSAGE_MAP()
/************************************************************************/
/*description: Constructor          
/*  arguments: void             
/*    return:  none                                          
/************************************************************************/
CXMLTreeView::CXMLTreeView()
{
	IsSelected = FALSE;
}
/************************************************************************/
/*description: Destructor  
/*  arguments: void             
/*    return:  none                                          
/************************************************************************/
CXMLTreeView::~CXMLTreeView()
{
}
BOOL CXMLTreeView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}
/************************************************************************/
/*description: update the view content.    
/*  arguments: pDC:  the device context provided by the system.             
/*    return:  void                                          
/************************************************************************/
void CXMLTreeView::OnDraw(CDC* pDC)
{
	CNodeManage *m_Manage = GetDocument()->m_pManage;
    //如果不为空的话
    if (m_Manage)
	{	
		m_Manage->showTree(this,pDC);
	}
}
/************************************************************************/
/*description: show and update the tree view graph. 
/*             
/*  arguments: set by the system.
/*               
/*    return:  void                                          
/************************************************************************/
void CXMLTreeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	Invalidate(TRUE);
}
void CXMLTreeView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	
	CSize sizeTotal;
	
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);
}
BOOL CXMLTreeView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CXMLTreeView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}
void CXMLTreeView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}
void CXMLTreeView::AssertValid() const
{
	CScrollView::AssertValid();
}
void CXMLTreeView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
CXMLTreeDoc* CXMLTreeView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CXMLTreeDoc)));
	return (CXMLTreeDoc*)m_pDocument;
}
void CXMLTreeView::OnLButtonDown(UINT nFlags, CPoint pt) 
{
	CNodeManage *Manage = GetDocument()->m_pManage;
	
	if (Manage)
	{
		if (Manage->clickTest(pt))
		{
			IsSelected = TRUE;
			start = pt;
		}
	}	
	CView::OnLButtonDown(nFlags, pt);
}
void CXMLTreeView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CNodeManage *Manage = GetDocument()->m_pManage;
    if (IsSelected)
    {
		Manage->offset(point.x-start.x,point.y-start.y);
		Invalidate(TRUE);
		start = point;
    }
	CView::OnMouseMove(nFlags, point);
}
void CXMLTreeView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (IsSelected)
	{
		IsSelected = NULL;
	}
	CView::OnLButtonUp(nFlags, point);
}
/************************************************************************/
/*description: show the context menu. 
/*             
/*  arguments: set by the system.
/*               
/*    return:  void                                          
/************************************************************************/
void CXMLTreeView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CMenu menu; 
	menu.LoadMenu(IDR_MENU_FIND); 
	CMenu* pPopup = menu.GetSubMenu(0); 
	ClientToScreen(&point);       //客户坐标转换成屏幕坐标 
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this); 
    
    
	CView::OnRButtonDown(nFlags, point);
}
/************************************************************************/
/*description: show the find node. 
/*             
/*  arguments: void.
/*               
/*    return:  void                                          
/************************************************************************/
void CXMLTreeView::find()
{
	CFindDlg findDlg;
	CString strStart;
	CString strEnd;
	BOOL    isFind;
	if (findDlg.DoModal()==IDOK)
	{
		strStart = findDlg.m_strStart;
		strEnd = findDlg.m_strEnd;
	}

	CNodeManage *m_Manage = GetDocument()->m_pManage;
    //如果不为空的话
    if (m_Manage)
		isFind = m_Manage->findNodes(strStart,strEnd);

	if (FALSE == isFind)
		MessageBox("Input Error,Not Found!");
	else
		Invalidate(TRUE);
}
/************************************************************************/
/*description: clear find path. 
/*             
/*  arguments: void.
/*               
/*    return:  void                                          
/************************************************************************/
void CXMLTreeView::clearFindPath()
{
	CNodeManage *m_Manage = GetDocument()->m_pManage;
    //如果不为空的话
    if (m_Manage)
	    m_Manage->clearFindPath();
	
	Invalidate(TRUE);
}
