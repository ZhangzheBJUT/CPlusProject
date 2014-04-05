; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CXMLTreeApp
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "XMLTree.h"
LastPage=0

ClassCount=8
Class1=CXMLTreeApp
Class2=CXMLTreeDoc
Class3=CTextNode
Class4=CMainFrame

ResourceCount=4
Resource1=IDR_MENU_FIND (Chinese (P.R.C.))
Class5=CAboutDlg
Class6=CFindDlg
Class7=CUniversalNode
Class8=CAttrNode
Resource2=IDD_ABOUTBOX
Resource3=IDD_DIALOG_FIND (Chinese (P.R.C.))
Resource4=IDR_MAINFRAME

[CLS:CXMLTreeApp]
Type=0
HeaderFile=XMLTree.h
ImplementationFile=XMLTree.cpp
Filter=N
LastObject=ID_MEUN_CLEAR_FIND_PATH

[CLS:CXMLTreeDoc]
Type=0
HeaderFile=XMLTreeDoc.h
ImplementationFile=XMLTreeDoc.cpp
Filter=N
BaseClass=CDocument
VirtualFilter=DC
LastObject=CXMLTreeDoc

[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
BaseClass=CFrameWnd
VirtualFilter=fWC
LastObject=CMainFrame




[CLS:CAboutDlg]
Type=0
HeaderFile=XMLTree.cpp
ImplementationFile=XMLTree.cpp
Filter=D
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_FILE_PRINT
Command6=ID_FILE_PRINT_PREVIEW
Command7=ID_FILE_PRINT_SETUP
Command8=ID_FILE_MRU_FILE1
Command9=ID_APP_EXIT
Command10=ID_EDIT_UNDO
Command11=ID_EDIT_CUT
Command12=ID_EDIT_COPY
Command13=ID_EDIT_PASTE
Command14=ID_VIEW_TOOLBAR
Command15=ID_VIEW_STATUS_BAR
Command16=ID_APP_ABOUT
Command17=ID_UPDATE
CommandCount=17

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
CommandCount=14

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
CommandCount=8

[CLS:CUniversalNode]
Type=0
HeaderFile=UniversalNode.h
ImplementationFile=UniversalNode.cpp
BaseClass=CButton
Filter=W
LastObject=CUniversalNode
VirtualFilter=BWC

[CLS:CAttrNode]
Type=0
HeaderFile=attrnode.h
ImplementationFile=attrnode.cpp
BaseClass=CUniversalNode
LastObject=CAttrNode
Filter=N

[MNU:IDR_MENU_FIND (Chinese (P.R.C.))]
Type=1
Class=?
Command1=IDC_FIND
Command2=ID_MEUN_CLEAR_FIND_PATH
CommandCount=2

[DLG:IDD_DIALOG_FIND (Chinese (P.R.C.))]
Type=1
Class=CFindDlg
ControlCount=7
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC_FIND,button,1342177287
Control4=IDC_STATIC_STARTNODE,static,1342308352
Control5=IDC_STATIC_ENDNODE,static,1342308352
Control6=IDC_EDIT_START,edit,1350631552
Control7=IDC_EDIT_END,edit,1350631552

[CLS:CFindDlg]
Type=0
HeaderFile=FindDlg.h
ImplementationFile=FindDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CFindDlg
VirtualFilter=dWC

