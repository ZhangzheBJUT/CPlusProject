/**************************************************************
*
*FileName: XMLTtreeView.h
*Creator : Zhangzhe
*   Date : 2012-11-19
*Comment : 用CElementNode、CAttrNode和CTextNode
*          对象图形显示XML的结构。
*
/*************************************************************/
#ifndef _XMLTREEVIEW_H
#define _XMLTREEVIEW_H

class CXMLTreeDoc;
class CUniversalNode;
class CNodeManage;

class CXMLTreeView : public CScrollView
{
protected: 
	CXMLTreeView();
	DECLARE_DYNCREATE(CXMLTreeView)
public: 
	CXMLTreeDoc* GetDocument();
	virtual ~CXMLTreeView();
private:
	BOOL   IsSelected;
	CPoint start;
public:
   
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXMLTreeView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	afx_msg void find();
	afx_msg void clearFindPath();
	//}}AFX_VIRTUAL

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	DECLARE_MESSAGE_MAP()
};
#endif

