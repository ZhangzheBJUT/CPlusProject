/**************************************************************
*
*FileName: XMLTextView.h
*Creator : Zhangzhe
*   Date : 2012-11-19
*Comment : ��ú���ʾ�û������롣
*
/*************************************************************/
#ifndef _XMLTEXTVIEW_H
#define _XMLTEXTVIEW_H

class CXMLTreeDoc;
class CXMLTextView : public CEditView
{
protected:
	CXMLTextView();         
	DECLARE_DYNCREATE(CXMLTextView)
public:
	CString m_strContent;   //������û�������
    void  getContent();     //����û�������
	CXMLTreeDoc* GetDocument();
	virtual ~CXMLTextView();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXMLTextView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL
// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	DECLARE_MESSAGE_MAP()
};
#endif
