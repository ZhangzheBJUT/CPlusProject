/**************************************************************
*
*FileName: XMLTreeDoc.h
*Creator : Zhangzhe
*   Date : 2012-11-19
*Comment : 存储、读取和组织管理数据
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
     CNodeManage *m_pManage;         //管理结点工具类      
     CString     m_strContent;        //存储用户输入
private:
	 CXMLParse    *m_pPrase;         //解析xml的工具类
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
