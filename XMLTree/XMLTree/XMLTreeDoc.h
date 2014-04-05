/**************************************************************
*
*FileName: XMLTreeDoc.h
*Creator : Zhangzhe
*   Date : 2012-11-19
*Comment : �洢����ȡ����֯��������
*
/*************************************************************/
#ifndef _XMLTREEDOC_H
#define _XMLTREEDOC_H

class CXMLParse;
class CNodeManage;
class CXMLTreeView;
class CXMLTextView;

class CXMLTreeDoc : public CDocument
{
protected:
	CXMLTreeDoc();
	DECLARE_DYNCREATE(CXMLTreeDoc)
public:
     CNodeManage *m_pManage;         //�����㹤����      
     CString     m_strContent;        //�洢�û�����
private:
	 CXMLParse    *m_pPrase;         //����xml�Ĺ�����
public:
	 BOOL generateNodes();
	 virtual ~CXMLTreeDoc();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXMLTreeDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:
	//{{AFX_MSG(CXMLTreeDoc)
	afx_msg void OnUpdateContent();
	afx_msg void OnFileOpen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
#endif //endif
