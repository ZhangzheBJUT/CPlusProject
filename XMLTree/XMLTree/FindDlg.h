/**************************************************************
*
*FileName: FindDlg.h
*Creator : Zhangzhe
*   Date : 2012-12-14
*Comment : ���ڻ���û�����Ĳ��ҶԻ���
/*************************************************************/
#ifndef _CFINDLG_H
#define _CFINDLG_H

class CFindDlg : public CDialog
{
public:
	CFindDlg(CWnd* pParent = NULL);   
	//{{AFX_DATA(CFindDlg)
	enum { IDD = IDD_DIALOG_FIND };
	CString	m_strStart;                 //��ʼ���
	CString	m_strEnd;                   //�������
	//}}AFX_DATA
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
protected:

	// Generated message map functions
	//{{AFX_MSG(CFindDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
#endif 
